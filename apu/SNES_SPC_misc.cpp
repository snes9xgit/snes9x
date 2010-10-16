// SPC emulation support: init, sample buffering, reset, SPC loading

// snes_spc 0.9.0. http://www.slack.net/~ant/

#include "SNES_SPC.h"

#include <string.h>

/* Copyright (C) 2004-2007 Shay Green. This module is free software; you
can redistribute it and/or modify it under the terms of the GNU Lesser
General Public License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version. This
module is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
details. You should have received a copy of the GNU Lesser General Public
License along with this module; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */

#include "blargg_source.h"

#define RAM         (m.ram.ram)
#define REGS        (m.smp_regs [0])
#define REGS_IN     (m.smp_regs [1])

// (n ? n : 256)
#define IF_0_THEN_256( n ) ((uint8_t) ((n) - 1) + 1)


//// Init

blargg_err_t SNES_SPC::init()
{
	memset( &m, 0, sizeof m );
	dsp.init( RAM );
	
	m.tempo = tempo_unit;
	
	// Most SPC music doesn't need ROM, and almost all the rest only rely
	// on these two bytes
	m.rom [0x3E] = 0xFF;
	m.rom [0x3F] = 0xC0;
	
	static unsigned char const cycle_table [128] =
	{//   01   23   45   67   89   AB   CD   EF
	    0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x68, // 0
	    0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x46, // 1
	    0x28,0x47,0x34,0x36,0x26,0x54,0x54,0x74, // 2
	    0x48,0x47,0x45,0x56,0x55,0x65,0x22,0x38, // 3
	    0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x66, // 4
	    0x48,0x47,0x45,0x56,0x55,0x45,0x22,0x43, // 5
	    0x28,0x47,0x34,0x36,0x26,0x44,0x54,0x75, // 6
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x36, // 7
	    0x28,0x47,0x34,0x36,0x26,0x54,0x52,0x45, // 8
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0xC5, // 9
	    0x38,0x47,0x34,0x36,0x26,0x44,0x52,0x44, // A
	    0x48,0x47,0x45,0x56,0x55,0x55,0x22,0x34, // B
	    0x38,0x47,0x45,0x47,0x25,0x64,0x52,0x49, // C
	    0x48,0x47,0x56,0x67,0x45,0x55,0x22,0x83, // D
	    0x28,0x47,0x34,0x36,0x24,0x53,0x43,0x40, // E
	    0x48,0x47,0x45,0x56,0x34,0x54,0x22,0x60, // F
	};
	
	// unpack cycle table
	for ( int i = 0; i < 128; i++ )
	{
		int n = cycle_table [i];
		m.cycle_table [i * 2 + 0] = n >> 4;
		m.cycle_table [i * 2 + 1] = n & 0x0F;
	}

	allow_time_overflow = false;
	
	#if SPC_LESS_ACCURATE
		memcpy( reg_times, reg_times_, sizeof reg_times );
	#endif
	
	reset();
	return 0;
}

void SNES_SPC::init_rom( uint8_t const in [rom_size] )
{
	memcpy( m.rom, in, sizeof m.rom );
}

void SNES_SPC::set_tempo( int t )
{
	m.tempo = t;
	int const timer2_shift = 4; // 64 kHz
	int const other_shift  = 3; //  8 kHz
	
	#if SPC_DISABLE_TEMPO
		m.timers [2].prescaler = timer2_shift;
		m.timers [1].prescaler = timer2_shift + other_shift;
		m.timers [0].prescaler = timer2_shift + other_shift;
	#else
		if ( !t )
			t = 1;
		int const timer2_rate  = 1 << timer2_shift;
		int rate = (timer2_rate * tempo_unit + (t >> 1)) / t;
		if ( rate < timer2_rate / 4 )
			rate = timer2_rate / 4; // max 4x tempo
		m.timers [2].prescaler = rate;
		m.timers [1].prescaler = rate << other_shift;
		m.timers [0].prescaler = rate << other_shift;
	#endif
}

// Timer registers have been loaded. Applies these to the timers. Does not
// reset timer prescalers or dividers.
void SNES_SPC::timers_loaded()
{
	int i;
	for ( i = 0; i < timer_count; i++ )
	{
		Timer* t = &m.timers [i];
		t->period  = IF_0_THEN_256( REGS [r_t0target + i] );
		t->enabled = REGS [r_control] >> i & 1;
		t->counter = REGS_IN [r_t0out + i] & 0x0F;
	}
	
	set_tempo( m.tempo );
}

// Loads registers from unified 16-byte format
void SNES_SPC::load_regs( uint8_t const in [reg_count] )
{
	memcpy( REGS, in, reg_count );
	memcpy( REGS_IN, REGS, reg_count );
	
	// These always read back as 0
	REGS_IN [r_test    ] = 0;
	REGS_IN [r_control ] = 0;
	REGS_IN [r_t0target] = 0;
	REGS_IN [r_t1target] = 0;
	REGS_IN [r_t2target] = 0;
}

// RAM was just loaded from SPC, with $F0-$FF containing SMP registers
// and timer counts. Copies these to proper registers.
void SNES_SPC::ram_loaded()
{
	m.rom_enabled = 0;
	load_regs( &RAM [0xF0] );
	
	// Put STOP instruction around memory to catch PC underflow/overflow
	memset( m.ram.padding1, cpu_pad_fill, sizeof m.ram.padding1 );
	memset( m.ram.padding2, cpu_pad_fill, sizeof m.ram.padding2 );
}

// Registers were just loaded. Applies these new values.
void SNES_SPC::regs_loaded()
{
	enable_rom( REGS [r_control] & 0x80 );
	timers_loaded();
}

void SNES_SPC::reset_time_regs()
{
	m.cpu_error     = 0;
	m.echo_accessed = 0;
	m.spc_time      = 0;
	m.dsp_time      = 0;
	#if SPC_LESS_ACCURATE
		m.dsp_time = clocks_per_sample + 1;
	#endif
	
	for ( int i = 0; i < timer_count; i++ )
	{
		Timer* t = &m.timers [i];
		t->next_time = 1;
		t->divider   = 0;
	}
	
	regs_loaded();
	
	m.extra_clocks = 0;
	reset_buf();
}

void SNES_SPC::reset_common( int timer_counter_init )
{
	int i;
	for ( i = 0; i < timer_count; i++ )
		REGS_IN [r_t0out + i] = timer_counter_init;
	
	// Run IPL ROM
	memset( &m.cpu_regs, 0, sizeof m.cpu_regs );
	m.cpu_regs.pc = rom_addr;
	
	REGS [r_test   ] = 0x0A;
	REGS [r_control] = 0xB0; // ROM enabled, clear ports
	for ( i = 0; i < port_count; i++ )
		REGS_IN [r_cpuio0 + i] = 0;
	
	reset_time_regs();
}

void SNES_SPC::soft_reset()
{
	reset_common( 0 );
	dsp.soft_reset();
}

void SNES_SPC::reset()
{
	m.cpu_regs.pc  = 0xFFC0;
	m.cpu_regs.a   = 0x00;
	m.cpu_regs.x   = 0x00;
	m.cpu_regs.y   = 0x00;
	m.cpu_regs.psw = 0x02;
	m.cpu_regs.sp  = 0xEF;
	memset( RAM, 0x00, 0x10000 );
	ram_loaded();
	reset_common( 0x0F );
	dsp.reset();
}

char const SNES_SPC::signature [signature_size + 1] =
		"SNES-SPC700 Sound File Data v0.30\x1A\x1A";

blargg_err_t SNES_SPC::load_spc( void const* data, long size )
{
	spc_file_t const* const spc = (spc_file_t const*) data;
	
	// be sure compiler didn't insert any padding into fle_t
	assert( sizeof (spc_file_t) == spc_min_file_size + 0x80 );
	
	// Check signature and file size
	if ( size < signature_size || memcmp( spc, signature, 27 ) )
		return "Not an SPC file";
	
	if ( size < spc_min_file_size )
		return "Corrupt SPC file";
	
	// CPU registers
	m.cpu_regs.pc  = spc->pch * 0x100 + spc->pcl;
	m.cpu_regs.a   = spc->a;
	m.cpu_regs.x   = spc->x;
	m.cpu_regs.y   = spc->y;
	m.cpu_regs.psw = spc->psw;
	m.cpu_regs.sp  = spc->sp;
	
	// RAM and registers
	memcpy( RAM, spc->ram, 0x10000 );
	ram_loaded();
	
	// DSP registers
	dsp.load( spc->dsp );
	
	reset_time_regs();
	
	return 0;
}

void SNES_SPC::clear_echo()
{
	if ( !(dsp.read( SPC_DSP::r_flg ) & 0x20) )
	{
		int addr = 0x100 * dsp.read( SPC_DSP::r_esa );
		int end  = addr + 0x800 * (dsp.read( SPC_DSP::r_edl ) & 0x0F);
		if ( end > 0x10000 )
			end = 0x10000;
		memset( &RAM [addr], 0xFF, end - addr );
	}
}


//// Sample output

void SNES_SPC::reset_buf()
{
	// Start with half extra buffer of silence
	sample_t* out = m.extra_buf;
	while ( out < &m.extra_buf [extra_size / 2] )
		*out++ = 0;
	
	m.extra_pos = out;
	m.buf_begin = 0;
	
	dsp.set_output( 0, 0 );
}

void SNES_SPC::set_output( sample_t* out, int size )
{
	require( (size & 1) == 0 ); // size must be even
	
	m.extra_clocks &= clocks_per_sample - 1;
	if ( out )
	{
		sample_t const* out_end = out + size;
		m.buf_begin = out;
		m.buf_end   = out_end;
		
		// Copy extra to output
		sample_t const* in = m.extra_buf;
		while ( in < m.extra_pos && out < out_end )
			*out++ = *in++;
		
		// Handle output being full already
		if ( out >= out_end )
		{
			// Have DSP write to remaining extra space
			out     = dsp.extra();
			out_end = &dsp.extra() [extra_size];
			
			// Copy any remaining extra samples as if DSP wrote them
			while ( in < m.extra_pos )
				*out++ = *in++;
			assert( out <= out_end );
		}
		
		dsp.set_output( out, out_end - out );
	}
	else
	{
		reset_buf();
	}
}

void SNES_SPC::save_extra()
{
	// Get end pointers
	sample_t const* main_end = m.buf_end;     // end of data written to buf
	sample_t const* dsp_end  = dsp.out_pos(); // end of data written to dsp.extra()
	if ( m.buf_begin <= dsp_end && dsp_end <= main_end )
	{
		main_end = dsp_end;
		dsp_end  = dsp.extra(); // nothing in DSP's extra
	}
	
	// Copy any extra samples at these ends into extra_buf
	sample_t* out = m.extra_buf;
	sample_t const* in;
	for ( in = m.buf_begin + sample_count(); in < main_end; in++ )
		*out++ = *in;
	for ( in = dsp.extra(); in < dsp_end ; in++ )
		*out++ = *in;
	
	m.extra_pos = out;
	assert( out <= &m.extra_buf [extra_size] );
}

blargg_err_t SNES_SPC::play( int count, sample_t* out )
{
	require( (count & 1) == 0 ); // must be even
	if ( count )
	{
		set_output( out, count );
		end_frame( count * (clocks_per_sample / 2) );
	}
	
	const char* err = m.cpu_error;
	m.cpu_error = 0;
	return err;
}

blargg_err_t SNES_SPC::skip( int count )
{
	#if SPC_LESS_ACCURATE
	if ( count > 2 * sample_rate * 2 )
	{
		set_output( 0, 0 );
		
		// Skip a multiple of 4 samples
		time_t end = count;
		count = (count & 3) + 1 * sample_rate * 2;
		end = (end - count) * (clocks_per_sample / 2);
		
		m.skipped_kon  = 0;
		m.skipped_koff = 0;
		
		// Preserve DSP and timer synchronization
		// TODO: verify that this really preserves it
		int old_dsp_time = m.dsp_time + m.spc_time;
		m.dsp_time = end - m.spc_time + skipping_time;
		end_frame( end );
		m.dsp_time = m.dsp_time - skipping_time + old_dsp_time;
		
		dsp.write( SPC_DSP::r_koff, m.skipped_koff & ~m.skipped_kon );
		dsp.write( SPC_DSP::r_kon , m.skipped_kon );
		clear_echo();
	}
	#endif
	
	return play( count, 0 );
}

//// Snes9x Accessor

void SNES_SPC::dsp_set_spc_snapshot_callback( void (*callback) (void) )
{
	dsp.set_spc_snapshot_callback( callback );
}

void SNES_SPC::dsp_dump_spc_snapshot( void )
{
	dsp.dump_spc_snapshot();
}

void SNES_SPC::dsp_set_stereo_switch( int value )
{
	dsp.set_stereo_switch( value );
}

SNES_SPC::uint8_t SNES_SPC::dsp_reg_value( int ch, int addr )
{
	return dsp.reg_value( ch, addr );
}

int SNES_SPC::dsp_envx_value( int ch )
{
	return dsp.envx_value( ch );
}

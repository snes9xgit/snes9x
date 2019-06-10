/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "CCGShader.h"
#include "../conffile.h"

CCGShader::CCGShader(void)
{

}

CCGShader::~CCGShader(void)
{
}

cgScaleType CCGShader::scaleStringToEnum(const char *scale)
{
	if(!strcasecmp(scale,"source")) {
		return CG_SCALE_SOURCE;
	} else if(!strcasecmp(scale,"viewport")) {
		return CG_SCALE_VIEWPORT;
	} else if(!strcasecmp(scale,"absolute")) {
		return CG_SCALE_ABSOLUTE;
	} else {
		return CG_SCALE_NONE;
	}
}

bool CCGShader::LoadShader(const char *path)
{
	ConfigFile conf;
	int shaderCount;
	char keyName[100];

	shaderPasses.clear();
	lookupTextures.clear();

	if(strlen(path)<4 || strcasecmp(&path[strlen(path)-4],".cgp")) {
		shaderPass pass;
		pass.scaleParams.scaleTypeX = CG_SCALE_NONE;
		pass.scaleParams.scaleTypeY = CG_SCALE_NONE;
		pass.linearFilter = false;
		pass.filterSet = false;
		strcpy(pass.cgShaderFile,path);
		shaderPasses.push_back(pass);
		return true;
	} else {
		conf.LoadFile(path);
	}	

	shaderCount = conf.GetInt("::shaders",0);

	if(shaderCount<1)
		return false;

	for(int i=0;i<shaderCount;i++) {
		shaderPass pass;
		sprintf(keyName,"::filter_linear%u",i);
		pass.linearFilter = conf.GetBool(keyName);
		pass.filterSet = conf.Exists(keyName);

		sprintf(keyName,"::scale_type%u",i);
		const char *scaleType = conf.GetString(keyName,"");
		if(!strcasecmp(scaleType,"")) {
			sprintf(keyName,"::scale_type_x%u",i);
			const char *scaleTypeX = conf.GetString(keyName,"");
			if(*scaleTypeX=='\0' && (i!=(shaderCount-1)))
				scaleTypeX = "source";
			pass.scaleParams.scaleTypeX = scaleStringToEnum(scaleTypeX);
			sprintf(keyName,"::scale_type_y%u",i);
			const char *scaleTypeY = conf.GetString(keyName,"");
			if(*scaleTypeY=='\0' && (i!=(shaderCount-1)))
				scaleTypeY = "source";
			pass.scaleParams.scaleTypeY = scaleStringToEnum(scaleTypeY);
		} else {
			cgScaleType sType = scaleStringToEnum(scaleType);
			pass.scaleParams.scaleTypeX = sType;
			pass.scaleParams.scaleTypeY = sType;
		}

		sprintf(keyName,"::scale%u",i);
		const char *scaleFloat = conf.GetString(keyName,"");
		int scaleInt = conf.GetInt(keyName,0);
		if(!strcasecmp(scaleFloat,"")) {
			sprintf(keyName,"::scale_x%u",i);
			const char *scaleFloatX = conf.GetString(keyName,"1.0");
			pass.scaleParams.scaleX = atof(scaleFloatX);
			pass.scaleParams.absX = conf.GetInt(keyName,1);
			sprintf(keyName,"::scale_y%u",i);
			const char *scaleFloatY = conf.GetString(keyName,"1.0");
			pass.scaleParams.scaleY = atof(scaleFloatY);
			pass.scaleParams.absY = conf.GetInt(keyName,1);

		} else {
			float floatval = atof(scaleFloat);
			pass.scaleParams.scaleX = floatval;
			pass.scaleParams.absX = scaleInt;
			pass.scaleParams.scaleY = floatval;
			pass.scaleParams.absY = scaleInt;
		}

		sprintf(keyName,"::shader%u",i);
		strcpy(pass.cgShaderFile,conf.GetString(keyName,""));

        sprintf(keyName,"::frame_count_mod%u",i);
        pass.frameCounterMod = conf.GetInt(keyName,0);

        sprintf(keyName,"::float_framebuffer%u",i);
        pass.floatFbo = conf.GetBool(keyName);

		shaderPasses.push_back(pass);
	}

	char *shaderIds = conf.GetStringDup("::textures","");

	char *id = strtok(shaderIds,";");
	while(id!=NULL) {
		lookupTexture tex;
		sprintf(keyName,"::%s",id);
		strcpy(tex.id,id);
		strcpy(tex.texturePath,conf.GetString(keyName,""));
		sprintf(keyName,"::%s_linear",id);
		tex.linearfilter = conf.GetBool(keyName,true);
		lookupTextures.push_back(tex);
		id = strtok(NULL,";");
	}

	free(shaderIds);

	return true;
}

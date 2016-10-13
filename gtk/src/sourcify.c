#include <stdio.h>
#include <sys/stat.h>

int
main (int argc,
      char *argv[])
{
    FILE *infile, *outfile;
    unsigned char inchar;
    unsigned int counter;
    struct stat file_info;

    if (argc != 4)
    {
        fprintf (stderr, "Usage: %s infile outfile identifier\n", argv[0]);
        return 1;
    }

    stat (argv[1], &file_info);

    infile = fopen (argv[1], "r");

    if (!infile)
    {
        fprintf (stderr, "Input file doesn't exist.\n");
        return 2;
    }

    outfile = fopen (argv[2], "w+");

    fprintf (outfile,
             "int %s_size = %d;\n\n",
             argv[3],
             (int) file_info.st_size);
    fprintf (outfile, "unsigned char %s [] = \n{\n  ", argv[3]);

    counter = 0;

    while (fread (&inchar, 1, 1, infile))
    {
        if (counter >= 32)
        {
            counter = 0;
            fprintf (outfile, "\n  ");
        }

        fprintf (outfile, "%d,", inchar);
        counter++;
    }

    /* Erase extra "," */
    fseek (outfile, -1, SEEK_CUR);

    fprintf (outfile, "\n};\n");

    fclose (infile);
    fclose (outfile);

    return 0;
}

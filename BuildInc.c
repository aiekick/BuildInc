#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LENGTH 1024
#define BUILD_WORD "BuildNumber"
#define MINOR_WORD "MinorNumber"
#define MAJOR_WORD "MajorNumber"
#define BUILD_ID_WORD "BuildId"

static int rule_maxBuildNumber = 1000;
static int rule_maxMinorNumber = 10;

int ParseRule(const char* vRule)
{
	if (vRule && strlen(vRule))
	{
		int maxBuild, maxMinor;
		int res = sscanf(vRule, "%i:%i", &maxBuild, &maxMinor);
		if (res == 2)
		{
			rule_maxBuildNumber = maxBuild;
			rule_maxMinorNumber = maxMinor;

			printf("Defined rule : %i:%i\n", rule_maxBuildNumber, rule_maxMinorNumber);

			return 1;
		}
	}

	printf("The rule is missing or wrong\n");
	printf("The rule must respect this format max_build_number:max_minor_number\n");
	printf("The default rule of 1000:10 will be applied\n");

	return 0;
}

void ParseFile(const char* vFile)
{
	int BuildNumber = 0;
	int MinorNumber = 0;
	int MajorNumber = 0;

	if (vFile && strlen(vFile))
	{
		int idx = 0;
		char bufLine[MAX_LENGTH + 1];

		FILE* fp = fopen(vFile, "r");
		if (fp)
		{
			// read
			while (!feof(fp) && idx < 3)
			{
				fgets(bufLine, MAX_LENGTH, fp);
				if (ferror(fp))
				{
					fprintf(stderr, "Reading error with code %d\n", errno);
					break;
				}

				char bufKey[255]; 
				bufKey[0] = '\0';
				
				int id = 0;
				int res = sscanf(bufLine, "#define %s %i", bufKey, &id);
				if (res == 2)
				{
					if (strcmp(bufKey, BUILD_WORD) == 0)
					{
						BuildNumber = id;
						idx++;
					}
					else if (strcmp(bufKey, MINOR_WORD) == 0)
					{
						MinorNumber = id;
						idx++;
					}
					else if (strcmp(bufKey, MAJOR_WORD) == 0)
					{
						MajorNumber = id;
						idx++;
					}
				}
			}
			fclose(fp);
		}

		if (idx == 3 || // parse du fichier ok 
			fp == NULL) // fichier non existant
		{
			// treatment
			BuildNumber++;
			if (BuildNumber > rule_maxBuildNumber)
			{
				BuildNumber = 0;
				MinorNumber++;
			}
			if (MinorNumber > rule_maxMinorNumber)
			{
				MinorNumber = 0;
				MajorNumber++;
			}
			
			// print vars :
			printf("Build Id : %i.%i.%i\n", MajorNumber, MinorNumber, BuildNumber);

			FILE* fp = fopen(vFile, "w");
			if (fp)
			{
				fputs("#pragma once\n\n", fp);

				// write
				int n = snprintf(bufLine, MAX_LENGTH, "#define %s %i\n", BUILD_WORD, BuildNumber);
				if (n)
				{
					fwrite(bufLine, n, 1, fp);
					n = snprintf(bufLine, MAX_LENGTH, "#define %s %i\n", MINOR_WORD, MinorNumber);
					if (n)
					{
						fwrite(bufLine, n, 1, fp);
						n = snprintf(bufLine, MAX_LENGTH, "#define %s %i\n", MAJOR_WORD, MajorNumber);
						if (n)
						{
							fwrite(bufLine, n, 1, fp);
							n = snprintf(bufLine, MAX_LENGTH, "#define %s \"%i.%i.%i\"\n", BUILD_ID_WORD, MajorNumber, MinorNumber, BuildNumber);
							if (n)
							{
								fwrite(bufLine, n, 1, fp);
							}
						}
					}
				}

				fclose(fp);
			}
		}
	}
}

int main(int argc, char* argv[]) // Don't forget first integral argument 'argc'
{
	/*
	the build id is : MajorNumber.MinorNumber.BuildNumber
	if the rule is "1000:10" this correspond to :
	if (BuildNumber > 1000)
		MinorNumber++
	if (MinorNumber > 10)
		MajorNumber++
	*/

#ifdef _DEBUG
	for (int i = 0; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	printf("\n");
#endif

	if (argc == 1)
	{
		printf("this func will increment in a c/c++ include file, 3 vars : MajorNumber, MinorNumber and BuildNumber, according to a rule\n");
		printf("the syntax is : BuildInc rule include_file\n");
		printf("the rule is 'max_build_number:max_minor_number' \n");
		printf("by ex with a rule of 1000:10 the corresponding pseudo code will be :\n");
		printf("if (BuildNumber > 1000) MinorNumber++;\n");
		printf("if (MinorNumber > 10) MajorNumber++\n");
		printf("the Build id will be MajorNumber.MinorNumber.BuildNumber\n");
	}

	int idx = 1;
	if (argc > 1)
	{
		int res = ParseRule(argv[idx]);
		if (res) idx++;
		if (argc > 2 || res == 0)
			ParseFile(argv[idx]);
	}

	return 0;
}



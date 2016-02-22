#include "labels.h"

static
label *_root;

const int ins_length = XSM_INSTRUCTION_SIZE * XSM_WORD_SIZE + 1; 

void
labels_reset ()
{
	label *ptr = _root, *next;
	
	while (ptr != NULL)
	{
		next = ptr->next;
		
		free(ptr->name);
		free(ptr);
		ptr = next;
	}
	
	_root = NULL;
}

int labels_phase_one(FILE *fp)
{
	char instruction[ins_length];
	char *label;
	int address = 0;
	
	fseek (fp, 0, SEEK_SET);
	while (fgets(instruction, ins_length, fp))
	{
		if (labels_is_label(instruction))
		{
			label = labels_get_name (instruction);
			labels_insert (label, address);
		}
		else
		{
			address = address + XSM_INSTRUCTION_SIZE;
		}
	}
	
	return TRUE; 
}

int
labels_is_label (const char *str)
{	
	if (strchr(str, ':'))
		return TRUE;
	return FALSE;
}

char*
labels_get_name (const char *label)
{
	const char *delim = ":";
	strtok (label, delim);
	
	return strdup(strtok(NULL, delim));
}

void
labels_insert (const char *label, int address)
{
	label *ptr;
	
	ptr = (label *) malloc (sizeof(label));
	
	ptr->name = label;
	ptr->address = address;
	
	ptr->next = _root;	
	_root = ptr;
	
	return;		
}

int
labels_phase_two (FILE *fin, FILE *fout, int base_address)
{
	char line[ins_length];
	char instr[XSM_WORD_SIZE];
	int address = 0;
	const char *s = " ,"
	char *opcode, *leftop, *rightop;
	
	fseek (fp, 0, SEEK_SET);
	while (fgets(line, ins_length, fp))
	{
		opcode = strtok (line, s);
		leftop = strtok (NULL, s);
		rightop = strtok (NULL, s);
		if (!strcasecmp (opcode, "JMP") || !strcasecmp(opcode, "CALL")
		{	
			rightop = leftop;
			leftop = "";
		}
		else if (!strcasecmp(opcode, "JNZ") || !strcasecmp(opcode, "JZ"))
		{
			strcat (leftop, ", "); 
		}
		if (labels_is_charstring(rightop))
		{
			address = labels_get_target (rightop);
			
			if (address < 0)
			{
				fprintf (stderr, "Can not resolve label %s.\n", rightop);
				return FALSE;
			}
			
			fprintf (fout, "%s %s%d\n", opcode,leftop, address + base_address);
		}
	}
	
	return TRUE;			
}

int
labels_is_charstring (char *str)
{
	char *p = str;
	
	while (*p)
	{
		if (isalpha(*p))
			return TRUE;
			
		p++;
	}
	
	return FALSE;
}

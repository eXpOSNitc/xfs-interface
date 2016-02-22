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
	char *label;
	int address = 0;
	
	fseek (fp, 0, SEEK_SET);
	while (fgets(line, ins_length, fp))
	{
		sscanf (line, "%s", instr);
		
		if (!strcasecmp(instr, "JMP"))
		{
			if 
		}
	}			
}



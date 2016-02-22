#ifndef LABELS_H
#define LABELS_H

typedef struct _label{
	char *name;
	int address;
	struct _label *next;
}label;

int labels_phase_one(FILE *fp);
int labels_write(FILE *fin, FILE *fout, int base_address);
#endif

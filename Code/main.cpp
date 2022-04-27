#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <string>
#include <cstdio>
#include "tomasulo.h"

using namespace std;

int cycle_addi = 0;
int cycle_addf = 0;
int cycle_mulf = 0;
int cycle_mem_exe = 0;
int cycle_mem_mem = 0;
int num_ROB = 0;
int num_addiRS = 0;
int num_addfRS = 0;
int num_mulfRS = 0;
int num_memRS = 0;
int num_addi = 0;
int num_addf = 0;
int num_mulf = 0;
int num_mem = 0;
bool memaccess = 0;

vector<mem_unit> memory;
Operation opcode;
item dst, src, tgt;
InstBuf ib;
IntARF *IntArf;
FpARF *FpArf;
IntRAT *IntRat;
FpRAT *FpRat;
ReOrderBuf *ROB;
RS *addiRS, *addfRS, *mulfRS, *memRS;
RS_entry e_m, m_w, w_c;
ADDIER *addier, *memer, *memer2;
ADDFER *addfer;
MULFER *mulfer;
vector<inst> instruction;
vector<inst> rom;
extern const int MAX_BUFFER = 65536;
const char *DELIM2 = ", \t()";
const char *DELIM3 = "RF";

int clk = 0;
void getinstruction(ifstream &);
void print(ofstream &);

void print(ofstream &ofile)
{
	cout << "The table of results is on \"output.txt\"." << endl;
	ofile << "	        	"
		  << "________________________________________________" << endl;
	ofile << "	        	"
		  << "ISSUE"
		  << "   "
		  << "EXECUTION"
		  << "   "
		  << "MEMORY"
		  << "   "
		  << "WRITE-BACK"
		  << "   "
		  << "COMMIT" << endl;
	ofile << "	        	"
		  << "________________________________________________" << endl;
	for (int j = 0; j < instruction.size(); j++)
	{
		ofile << instruction[j].name
			  << "              "
			  << instruction[j].t_issue
			  << "	    "
			  << instruction[j].t_ex
			  << "	      ";
		if (instruction[j].t_mem != 0)
			ofile << instruction[j].t_mem << "      	 ";
		else
			ofile << "      	 ";
		ofile << instruction[j].t_wb << "	    " << instruction[j].t_commit << endl;
		ofile << endl;

	}
	ofile << endl;
	ofile << "_____________________________" << endl;
	ofile << "Content of the register file:" << endl;
	ofile << "_____________________________" << endl;
	ofile << endl;

	for (int j = 0; j < 16; j++)
	{
		ofile << "R" << j << " = " << IntArf->table[j].value << "    ";
		if (j == 15)
			ofile << '\n';
	}
	ofile << endl;
	for (int j = 0; j < 16; j++)
	{
		ofile << "F" << j << " = " << FpArf->table[j].value << "    ";
		if (j == 15)
			ofile << '\n';
	}
	ofile << '\n';
	ofile << "______________________" << endl;
	ofile << "Content of the memory:" << endl;
	ofile << "______________________" << endl;
	ofile << endl;

	for (int j = 0; j < memory.size(); j++)
		ofile << "Mem[" << memory[j].first << "]=" << memory[j].second << "	";
	ofile << '\n';
}
void getinstruction(ifstream &infile)
{
	char buf[30];
	string buf1;
	string rs_tmp;
	while (infile.getline(buf, 65535))
	{
		buf1 = buf;
		char *word;
		vector<char *> words;
		Operation opcode;
		string op;
		string token, label, rd, rs, rt;
		int i_rs = 0, i_rt = 0, i_rd = 0, i_label = 0;
		word = strtok(buf, DELIM2);
		if (word == NULL)
			continue;
		while (word)
		{
			words.push_back(word);
			word = strtok(NULL, DELIM2);
		}
		op = words[0];
		string rs_tmp;
		if (op.compare("Sd") == 0)
		{
			rs = words[1];
			rs_tmp = words[2];
			rt = words[3];
			opcode = store;
		}
		else if (op.compare("Ld") == 0)
		{
			opcode = load;
			rd = words[1];
			rs = words[2];
			rt = words[3];
		}
		else if (op.compare("Beq") == 0 || op.compare("Bne") == 0)
		{
			rs = words[1];	 
			rt = words[2];	 
			rs_tmp = words[3]; 
			if (op.compare("Beq") == 0)
				opcode = beq;
			else
				opcode = bne;
		}
		else if (op.compare("Add") == 0)
		{
			rd = words[1];
			rs = words[2];
			rt = words[3];
			opcode = addi;
		}
		else if (op.compare("Sub") == 0)
		{
			rd = words[1];
			rs = words[2];
			rt = words[3];
			opcode = subi;
		}
		else if (op.compare("Subf") == 0)
		{
			rd = words[1];
			rs = words[2];
			rt = words[3];
			opcode = subf;
		}
		else if (op.compare("Addf") == 0)
		{

			rd = words[1];
			rs = words[2];
			rt = words[3];
			opcode = addf;
		}
		else if (op.compare("Mulf") == 0)
		{
			opcode = mulf;
			rd = words[1];
			rs = words[2];
			rt = words[3];
		}
		else
		{
			cout << "Not a valid operation code:" << op << endl;
			continue;
		}

		char a[4];

		rd.copy(a, 3);
		if (a[0] == 'R')
		{

			dst.id = atoi(&rd.c_str()[1]) + 1;
			dst.imme_flag = 0;
			dst.ready = 0;
		}
		else if (a[0] == 'F')
		{
			dst.id = atoi(&rd.c_str()[1]) + 101;
			dst.imme_flag = 0;
			dst.ready = 0;
		}

		rs.copy(a, 3);

		if (a[0] == 'R')
		{
			src.id = atoi(&rs.c_str()[1]) + 1;
			src.value = 0;
			src.ready = 0;
			src.imme_flag = 0;
		}
		else if (a[0] == 'F')
		{
			src.id = atoi(&rs.c_str()[1]) + 101;
			src.value = 0;
			src.ready = 0;
			src.imme_flag = 0;
		}
		else
		{
			src.value = atof(rs.c_str());
			src.imme_flag = 1;
			src.ready = 1;
		}

		rt.copy(a, 3);

		if (a[0] == 'R')
		{
			tgt.id = atoi(&rt.c_str()[1]) + 1;
			tgt.value = 0;
			tgt.ready = 0;
			tgt.imme_flag = 0;
		}
		else if (a[0] == 'F')
		{
			tgt.id = atoi(&rt.c_str()[1]) + 101;
			tgt.value = 0;
			tgt.ready = 0;
			tgt.imme_flag = 0;
		}
		else
		{
			tgt.value = atof(rt.c_str());
			tgt.imme_flag = 1;
			tgt.ready = 1;
		}
		inst itmp;
		itmp.name = buf1;
		itmp.opcode = opcode;
		if (itmp.opcode == store || itmp.opcode == bne || itmp.opcode == beq)
			itmp.rd.id = 0;
		else
			itmp.rd = dst;
		itmp.rs = src;
		itmp.rt = tgt;
		itmp.sd_offset = rs_tmp;
		instruction.push_back(itmp);
	}
}


int main(int argc, char *argv[])
{
	cout << "Tomasulo Simulator" << endl;
	cout << "------------------" << endl;
	bool first = 1;
	ifstream infile;
	infile.open("input-instructions.txt", ios::in);
	if (infile.fail())
	{
		cout << "Can't open input-instruction.txt" << endl;
		exit(-1);
	}
	ofstream outfile;
	outfile.open("output.txt", ios::out);

	initial();
	getinstruction(infile);
	rom = instruction;

	int tmp = instruction.size();
	while (true)
	{
		clk++;
		if (instruction[instruction.size() - 1].t_commit != 0)
		{
			print(outfile);
			outfile.close();
			delete IntArf;
			delete FpArf;
			delete IntRat;
			delete FpRat;
			delete ROB;
			delete addiRS;
			delete addfRS;
			delete mulfRS;
			delete memRS;
			delete[] addier;
			delete[] addfer;
			delete[] mulfer;
			return 0;
		}
		commit();
		writeback();
		mem();
		execution();
		issue();
		if (first == 1)
			first = 0;
	}
	return 0;
}

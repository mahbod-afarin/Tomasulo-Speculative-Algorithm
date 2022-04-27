#include<fstream>
#include<vector>
#include"tomasulo.h"
using namespace std;

extern RS_entry e_m;
extern RS_entry m_w;
extern RS_entry w_c;
extern vector<inst> instruction;
extern vector<inst> rom;
extern ADDIER *memer2;
extern bool memaccess;

extern RS_entry e_m;
extern RS_entry m_w;
extern RS_entry w_c;
vector<RS_entry> cmt_vector;

int pc=0;
int last_pc=-1;
bool issue_flag=false;

const int num_iR=36;			
const int num_fpR=36;			
extern int num_ROB;			

extern int num_addiRS;
extern int num_addfRS;
extern int num_mulfRS;
extern int num_memRS;

extern int cycle_addi;
extern int cycle_addf;
extern int cycle_mulf;
extern int cycle_mem_exe;
extern int cycle_mem_mem;

extern int num_addi;
extern int num_addf;
extern int num_mulf;
extern int num_mem;

extern Operation opcode;
extern item dst,src,tgt;
extern InstBuf ib;
extern IntARF *IntArf;
extern FpARF *FpArf;
extern IntRAT *IntRat;
extern FpRAT *FpRat;
extern ReOrderBuf *ROB;
extern RS *addiRS,*addfRS,*mulfRS,*memRS;
extern ADDIER *addier, *memer, *memer2;
extern ADDFER *addfer;
extern MULFER *mulfer;
extern vector<inst> instruction;

extern int clk;		

const int MAX_BUFFER = 65535;
const char * DELIM0=" \t";
const char * DELIM1=" =,[]";
extern vector<mem_unit> memory;

IntARF::IntARF(int N)
{
	table = new ARF<int>[N];
	for(int j=0;j<N;j++){
		table[j].id=j+1;
		table[j].ready=0;
		table[j].value=0;
	}
	pointer=0;
}

FpARF::FpARF(int N)
{
	table = new ARF<float>[N];
	for(int j=0;j<N;j++){
		table[j].id=(j+101);
		table[j].ready=0;
		table[j].value=0.0;
	}
	pointer=0;
}

IntRAT::IntRAT(int N)
{
	table = new RAT[N];
	for(int j=0;j<N;j++) {
		table[j].alias = IntArf->table[j].id;
		table[j].value = IntArf->table[j].value;
	}
	pointer=0;
}

FpRAT::FpRAT(int N)
{
	table = new RAT[N];
	for(int j=0;j<N;j++) {
		table[j].alias = FpArf->table[j].id;
		table[j].value = FpArf->table[j].value;
	}
	pointer=0;
}

ReOrderBuf::ReOrderBuf(int N)
{
	table = new ReOrderBuf_entry[N];
	for(int j=0;j<N;j++){
		table[j].id=j+201;
		table[j].dst_id=0;
		table[j].value=0.0;
		table[j].cmt_flag=0;
	}
	head=0;
	tail=0;
	n = N;
	size=0;
}
int ReOrderBuf::get_size()
{
	return size;
}
bool ReOrderBuf::empty(){
	if (size== 0) return true;
	else return false;
}
bool ReOrderBuf::full() {
	if(size==n) return true;
	else return false;
}

item::item()
{
	value=0.0;
	id=0;
	imme_flag=0;
	ready=0;
}

RS::RS(int N)
{
	table= new RS_entry[N];
	head=tail=0;
	n = N;
	size=0;
}
int RS::get_size()
{
	return size;
}
bool RS::empty(){
	if (size== 0) return true;
	else return false;
}
bool RS::full() {
	if(size==n) return true;
	else return false;
}
ADDIER::ADDIER()
{
	cycle = cycle_addi;
	empty = 1;
}

ADDFER::ADDFER()
{
	line = new RS_entry[cycle_addf];
	head=tail= 0;
	cycle = cycle_addf;
	n=cycle;
	size=0;
}
int ADDFER::get_size()
{
	return size;
}
bool ADDFER::empty(){
	if (size== 0) return true;
	else return false;
}
bool ADDFER::full() {
	if(size==n) return true;
	else return false;
}
MULFER::MULFER()
{
	line = new RS_entry[cycle_mulf];
	head=tail= 0;
	cycle = cycle_mulf;
	n=cycle;
	size=0;
}
int MULFER::get_size()
{
	return size;
}
bool MULFER::empty(){
	if (size== 0) return true;
	else return false;
}
bool MULFER::full() {
	if(size==n) return true;
	else return false;
}
void initial()
{
	ifstream config;
	config.open("configuration.txt");

	src.value=0.0;
	src.imme_flag=0;
	tgt.value=0;
	tgt.imme_flag=0;
	ib.free=1;

	if (config.fail()) {
		cout << "Can't open configuration.txt file" << endl;
		exit(-1);
	}

	char buf[MAX_BUFFER];
	config.getline(buf, MAX_BUFFER);
	for (int i = 0; i <= 3; i++) {
		config.getline(buf, MAX_BUFFER);
		char * word;
		vector<char *> words;
		string token, token1;
		word = strtok(buf, DELIM0);
		while (word) {
			words.push_back(word);
			word = strtok(NULL, DELIM0);
		}

		token = words[0];
		if ((token.at(0) == 'A') || (token.at(0) == 'a')) {
			token1 = words[1];
			if ((token1.at(0) == 'I') || (token1.at(0) == 'i')) {
				num_addiRS=atoi(words[2]);
				cycle_addi=atoi(words[3]);
				num_addi=atoi(words[4]);
			}
		} else if ((token.at(0) == 'F') || (token.at(0) == 'f')) {
			token1 = words[1];
			if ((token1.at(0) == 'A') || (token1.at(0) == 'a')) {
				num_addfRS=atoi(words[2]);
				cycle_addf=atoi(words[3]);
				num_addf=atoi(words[4]);
			}
			else {
				num_mulfRS=atoi(words[2]);
				cycle_mulf=atoi(words[3]);
				num_mulf=atoi(words[4]);
			}
		} else {
			num_memRS=atoi(words[2]);
			cycle_mem_exe=atoi(words[3]);
			cycle_mem_mem=atoi(words[4]);
			num_mem=atoi(words[5]);
		}
	}

	IntArf = new IntARF(num_iR);
	
	
	FpArf = new FpARF(num_fpR);
	FpRat = new FpRAT(num_fpR);


	addiRS = new RS(num_addiRS);
	addfRS = new RS(num_addfRS);
	mulfRS = new RS(num_mulfRS);
	memRS = new RS(num_memRS);
	addier = new ADDIER[num_addi];
	memer= new ADDIER;
	memer2= new ADDIER;
	addfer = new ADDFER[num_addf];
	mulfer = new MULFER[num_mulf];

	config.getline(buf, MAX_BUFFER);
	for (int i = 0; i <=2; i++) {
		config.getline(buf, MAX_BUFFER);
		char * word;
		vector<char *> words;
		string token, token1;
		word = strtok(buf, DELIM1);
		while (word) {
			words.push_back(word);
			word = strtok(NULL, DELIM1); 
		}
		token = words[0];
		if ((token.at(0) == 'R') && (token.at(1) == 'O') && (token.at(2) == 'B'))  {
			num_ROB=atoi(words[2]);
		} else if (((token.at(0) == 'R') && (token.at(1) != 'O')) || (token.at(0) == 'F')) {
			int k = 0;
			int max = words.size();
			while (token.at(0) == 'R') {
				int index;
				index = token.at(1) - '0';
				IntArf->table[index].value = atoi(words[1+k]);
				k = k + 2;
				if (k <= max-1)	token = words[k];
				else break;
			}
			while (token.at(0) == 'F') {
				int index;
				float fp_value;
				index = token.at(1) - '0';
				if(token.at(2)!=0)index=index*10+token.at(2) - '0';
				FpArf->table[index].value = atof(words[1+k]);
				k = k + 2;
				if (k <= max-1)	token = words[k];
				else break;
			}
		} else {
			int k = 0;
			int max = words.size();
			while (token.at(0) == 'M') {
				int index;
				index = atoi(words[1+k]);
				mem_unit memtmp=make_pair(index,atof(words[2+k]));
				memory.push_back(memtmp);
				k = k + 3;
				if (k <= max-2) token = words[k];
				else break;
			}
		}
	}
	config.close();

	FpRat = new FpRAT(num_fpR);
	IntRat = new IntRAT(num_iR);
	ROB = new ReOrderBuf(num_ROB);
}

//issue

void rename()
{
	issue_flag = true;
		if(instruction[pc].rs.imme_flag==0){
			int tmp=instruction[pc].rs.id;
			if(instruction[pc].rs.id>=1&&instruction[pc].rs.id<=100){ 
				instruction[pc].rs.id=IntRat->table[instruction[pc].rs.id-1].alias;
				
				if(instruction[pc].rs.id==tmp){ 
					instruction[pc].rs.value=IntRat->table[instruction[pc].rs.id-1].value;
					instruction[pc].rs.ready=1;
				}
			}
			else if(instruction[pc].rs.id>=101&&instruction[pc].rs.id<=200){
				instruction[pc].rs.id=FpRat->table[instruction[pc].rs.id-101].alias;
				if(instruction[pc].rs.id==tmp){
					instruction[pc].rs.value=FpRat->table[instruction[pc].rs.id-101].value;
					instruction[pc].rs.ready=1;
				}
			}
		}
		if(instruction[pc].rt.imme_flag==0){
			int tmp=instruction[pc].rt.id;
			if(instruction[pc].rt.id>=1&&instruction[pc].rt.id<=100){ 
				instruction[pc].rt.id=IntRat->table[instruction[pc].rt.id-1].alias;
				if(instruction[pc].rt.id==tmp){ 
					instruction[pc].rt.value=IntRat->table[instruction[pc].rt.id-1].value;
					instruction[pc].rt.ready=1;
				}
			}
			else if(instruction[pc].rt.id>=101&&instruction[pc].rt.id<=200){
				instruction[pc].rt.id=FpRat->table[instruction[pc].rt.id-101].alias;
				if(instruction[pc].rt.id==tmp){
					instruction[pc].rt.value=FpRat->table[instruction[pc].rt.id-101].value;
					instruction[pc].rt.ready=1;
				}
			}
		}
		ROB->table[ROB->tail].dst_id=instruction[pc].rd.id;
		int tmp=instruction[pc].rd.id;
		instruction[pc].rd.id=ROB->table[ROB->tail].id;
		ROB->tail=(ROB->tail+1)%num_ROB;
		ROB->size++;
		
		if(tmp>=1&&tmp<=100)IntRat->table[tmp-1].alias=instruction[pc].rd.id;
		else if(tmp>=101&&tmp<=200) FpRat->table[tmp-101].alias=instruction[pc].rd.id;
}


void issue()
{
	if((last_pc!=pc)&&(!ROB->full())&&pc<instruction.size())
	{	


		if((!memRS->full())&&(instruction[pc].opcode == load||instruction[pc].opcode == store)){	
				rename();

				memRS->table[memRS->tail].d_r=instruction[pc].rd;
				memRS->table[memRS->tail].s_r=instruction[pc].rs;
				memRS->table[memRS->tail].t_r=instruction[pc].rt;
				memRS->table[memRS->tail].time=clk;
				memRS->table[memRS->tail].icount=pc;
				instruction[pc].t_issue=clk;
				memRS->tail=(memRS->tail+1)%num_memRS;
				memRS->size++;
		} 
		if((!mulfRS->full())&&(instruction[pc].opcode == mulf)){
				rename();

				mulfRS->table[mulfRS->tail].d_r=instruction[pc].rd;
				mulfRS->table[mulfRS->tail].s_r=instruction[pc].rs;
				mulfRS->table[mulfRS->tail].t_r=instruction[pc].rt;
				mulfRS->table[mulfRS->tail].time=clk;
				mulfRS->table[mulfRS->tail].icount=pc;
				instruction[pc].t_issue=clk;
				mulfRS->tail=(mulfRS->tail+1)%num_mulfRS;
				mulfRS->size++;
		}
		if((!addfRS->full())&&(instruction[pc].opcode == addf||instruction[pc].opcode == subf)){
				rename();

				addfRS->table[addfRS->tail].d_r=instruction[pc].rd;
				addfRS->table[addfRS->tail].s_r=instruction[pc].rs;
				addfRS->table[addfRS->tail].t_r=instruction[pc].rt;
				addfRS->table[addfRS->tail].time=clk;
				addfRS->table[addfRS->tail].icount=pc;
				instruction[pc].t_issue=clk;
				addfRS->tail=(addfRS->tail+1)%num_addfRS;
				addfRS->size++;
		}
		if((!addiRS->full())&&(instruction[pc].opcode == addi||instruction[pc].opcode == subi||instruction[pc].opcode == beq||instruction[pc].opcode == bne)){
				rename();

				addiRS->table[addiRS->tail].d_r=instruction[pc].rd;
				addiRS->table[addiRS->tail].s_r=instruction[pc].rs;
				addiRS->table[addiRS->tail].t_r=instruction[pc].rt;
				addiRS->table[addiRS->tail].time=clk;
				addiRS->table[addiRS->tail].icount=pc;
				instruction[pc].t_issue=clk;
				addiRS->tail=(addiRS->tail+1)%num_addiRS;
				addiRS->size++;
		}
		if(issue_flag){
			issue_flag = false;
			last_pc = pc;
			if(pc<instruction.size()&&(instruction[pc].opcode!=bne&&instruction[pc].opcode!=beq)) pc++;
		}
	}
}

//execution

void execution()
{
		Load();
		Mulf();
		Addf();
		Addi();
}

void Addi()
{
	for(int j=0;j<num_addi;j++){
		int temp_offset;
		if(!addiRS->empty()){
			if((addier[j].empty==1)&&(addiRS->table[addiRS->head].s_r.ready==1)&&(addiRS->table[addiRS->head].t_r.ready==1)){
				addier[j].line=addiRS->table[addiRS->head];
				addier[j].empty=0;
				addier[j].line.time=clk;
				addiRS->head=(addiRS->head+1)%num_addiRS;
				addiRS->size--;
				
				if(instruction[addier[j].line.icount].opcode==addi)addier[j].line.d_r.value=addier[j].line.s_r.value+addier[j].line.t_r.value;

				else if(instruction[addier[j].line.icount].opcode==subi)addier[j].line.d_r.value=addier[j].line.s_r.value-addier[j].line.t_r.value;

				else if(instruction[addier[j].line.icount].opcode==bne){
					if(addier[j].line.s_r.value-addier[j].line.t_r.value!=0) {
						temp_offset = atoi(instruction[addier[j].line.icount].sd_offset.c_str());

						if(temp_offset<0){
							temp_offset = -temp_offset;
							vector<inst> tempv_inst(temp_offset+1);
							vector<inst>::iterator it_instruction = instruction.begin();
							vector<inst>::iterator it_rom = rom.begin();
							copy(it_rom+pc-temp_offset,it_rom+pc+1,tempv_inst.begin());
							rom.insert(it_rom+pc+1,tempv_inst.begin(),tempv_inst.end());
							instruction.insert(it_instruction+pc+1,tempv_inst.begin(),tempv_inst.end());
							pc++;
						}

						else pc += temp_offset;
					}
					else pc++;
				}
				else if(instruction[addier[j].line.icount].opcode==beq){
					if(addier[j].line.s_r.value-addier[j].line.t_r.value==0) {
						temp_offset = atoi(instruction[addier[j].line.icount].sd_offset.c_str());
						if(temp_offset<0){
							temp_offset = -temp_offset;
							vector<inst> tempv_inst(temp_offset+1);
							vector<inst>::iterator it_instruction = instruction.begin();
							vector<inst>::iterator it_rom = rom.begin();
							copy(it_rom+pc-temp_offset,it_rom+pc+1,tempv_inst.begin());
							rom.insert(it_rom+pc+1,tempv_inst.begin(),tempv_inst.end());
							instruction.insert(it_instruction+pc+1,tempv_inst.begin(),tempv_inst.end());
							pc++;
						}
						else pc += temp_offset;
					}
					else pc++;
				}
				instruction[addier[j].line.icount].t_ex=clk;
			}
		}
		if(m_w.empty==1&&(addier[j].empty!=1)&&(clk>=(addier[j].line.time+cycle_addi-1))){
				m_w=addier[j].line;
				m_w.empty=0;
				instruction[addier[j].line.icount].t_mem=0;
				memaccess = 0;
				addier[j].empty=1;
		}
	}
}

void Addf()
{
	for(int j=0;j<num_addf;j++){
		if(!addfRS->empty()){		
			if((!addfer[j].full())&&(addfRS->table[addfRS->head].s_r.ready==1)&&(addfRS->table[addfRS->head].t_r.ready==1)){
				addfer[j].line[addfer[j].tail]=addfRS->table[addfRS->head];
				addfer[j].line[addfer[j].tail].time=clk;
				addfRS->head=(addfRS->head+1)%num_addfRS;
				addfRS->size--;
				
				if(instruction[addfer[j].line[addfer[j].tail].icount].opcode==addf) addfer[j].line[addfer[j].tail].d_r.value=addfer[j].line[addfer[j].tail].s_r.value+addfer[j].line[addfer[j].tail].t_r.value;
				else addfer[j].line[addfer[j].tail].d_r.value=addfer[j].line[addfer[j].tail].s_r.value-addfer[j].line[addfer[j].tail].t_r.value;
				
				addfer[j].line[addfer[j].tail].d_r.ready=1;
				instruction[addfer[j].line[addfer[j].tail].icount].t_ex=clk;
				addfer[j].tail=(addfer[j].tail+1)%cycle_addf;
				addfer[j].size++;
			}
		}

		if((m_w.empty==1)&&(!addfer[j].empty())&&(clk>=(addfer[j].line[addfer[j].head].time+cycle_addf-1))){
			m_w=addfer[j].line[addfer[j].head];
				m_w.empty=0;
				instruction[addier[j].line.icount].t_mem=0;
				memaccess = 0;
				addfer[j].head=(addfer[j].head+1)%cycle_addf;
				addfer[j].size--;				
		}
	}
}

void Mulf()
{
	for(int j=0;j<num_addf;j++){
		if(!mulfRS->empty()){
			if(((mulfer[j].tail+1)%cycle_mulf!=mulfer[j].head)&&(mulfRS->table[mulfRS->head].s_r.ready==1)&&(mulfRS->table[mulfRS->head].t_r.ready==1)){
				mulfer[j].line[mulfer[j].tail]=mulfRS->table[mulfRS->head];
				mulfer[j].line[mulfer[j].tail].time=clk;
				mulfRS->head=(mulfRS->head+1)%num_mulfRS;
				mulfRS->size--;
				mulfer[j].line[mulfer[j].head].d_r.value=mulfer[j].line[mulfer[j].head].s_r.value*mulfer[j].line[mulfer[j].head].t_r.value;
				mulfer[j].line[mulfer[j].tail].d_r.ready=1;
				instruction[mulfer[j].line[mulfer[j].tail].icount].t_ex=clk;
				mulfer[j].tail=(mulfer[j].tail+1)%cycle_mulf;
				mulfer[j].size++;
			}
		}

		if((clk>=(mulfer[j].line[mulfer[j].head].time+cycle_mulf-1)&&(m_w.empty==true)&&(mulfer[j].head!=mulfer[j].tail))){
			m_w=mulfer[j].line[mulfer[j].head];
			m_w.empty=0;
			instruction[addier[j].line.icount].t_mem=0;
			memaccess = 0;
			mulfer[j].head=(mulfer[j].head+1)%cycle_mulf;
			mulfer[j].size--;
		}
	}
}

void Load()
{
	if(!memRS->empty()){
		if((memer->line.empty)&&(memRS->table[memRS->head].s_r.ready==1)&&(memRS->table[addiRS->head].t_r.ready==1)){
				memer->line=memRS->table[memRS->head];
				memer->empty=0;
				memer->line.time=clk;
				memRS->head=(memRS->head+1)%num_memRS;
				memRS->size--;
				
				if(instruction[memer->line.icount].opcode==load){
					for(int j=0;j<memory.size();j++){
						if(memory[j].first==memer->line.s_r.value+memer->line.t_r.value)memer->line.d_r.value=memory[j].second;
					}
				}
				else{	
					int fg=0;
					mem_unit memtmp=make_pair(atoi(instruction[memer->line.icount].sd_offset.c_str())+memer->line.t_r.value,memer->line.s_r.value);
					for(int i=0;i<memory.size();i++){
						if(memory[i].first==memtmp.first) memory[i].second = memtmp.second;
						fg=1;
					}
						if(fg==0)memory.push_back(memtmp);												
				}
				instruction[memer->line.icount].t_ex=clk;
				
			}
		}
	if(e_m.empty==1&&(memer->empty!=1)&&(clk>=(memer->line.time+cycle_mem_exe-1))){
				e_m=memer->line;
				e_m.empty=0;
				memaccess = 1;
				memer->empty=1;
		}
}

//memory

void mem()
{
	if(e_m.empty==0){
		if(instruction[e_m.icount].opcode == load || instruction[e_m.icount].opcode == store){
			if(memer2->empty==1){
				memer2->line=e_m;
				memer2->empty=0;
				memer2->line.time=clk;
				e_m.empty=1;
				instruction[memer2->line.icount].t_mem=clk;
			}
		}else if(m_w.empty==1){
			m_w=e_m;
			m_w.time=clk;
			instruction[e_m.icount].t_mem=clk;
			m_w.empty=0;
			e_m.empty=1;
		}
	}
	if((memer2->empty!=1)&&(clk>=(memer2->line.time+cycle_mem_mem-1))&&m_w.empty==1){
			m_w=memer2->line;
			m_w.empty=0;
			memaccess = 0;
			memer2->empty=1;
	}
}

//write-back

void writeback()
{
	
	if(m_w.empty==0&&w_c.empty==1){
		m_w.time=clk;
		for(int j=0;j<num_ROB;j++){
			if(ROB->table[j].id==m_w.d_r.id)ROB->table[j].value=m_w.d_r.value;
		}
		for(int j=0;j<num_addiRS;j++){
			if(addiRS->table[j].s_r.id==m_w.d_r.id){
				addiRS->table[j].s_r.value=m_w.d_r.value;
				addiRS->table[j].s_r.ready=1;
			}
			if(addiRS->table[j].t_r.id==m_w.d_r.id){
				addiRS->table[j].t_r.value=m_w.d_r.value;
				addiRS->table[j].t_r.ready=1;
			}
		}
		for(int j=0;j<num_addfRS;j++){
			if(addfRS->table[j].s_r.id==m_w.d_r.id){
				addfRS->table[j].s_r.value=m_w.d_r.value;
				addfRS->table[j].s_r.ready=1;
			}
			if(addfRS->table[j].t_r.id==m_w.d_r.id){
				addfRS->table[j].t_r.value=m_w.d_r.value;
				addfRS->table[j].t_r.ready=1;
			}
		}
		for(int j=0;j<num_mulfRS;j++){
			if(mulfRS->table[j].s_r.id==m_w.d_r.id){
				mulfRS->table[j].s_r.value=m_w.d_r.value;
				mulfRS->table[j].s_r.ready=1;
			}
			if(mulfRS->table[j].t_r.id==m_w.d_r.id){
				mulfRS->table[j].t_r.value=m_w.d_r.value;
				mulfRS->table[j].t_r.ready=1;
			}
		}
		for(int j=0;j<num_memRS;j++){
			if(memRS->table[j].s_r.id==m_w.d_r.id){
				memRS->table[j].s_r.value=m_w.d_r.value;
				memRS->table[j].s_r.ready=1;
			}
			if(memRS->table[j].t_r.id==m_w.d_r.id){
				memRS->table[j].t_r.value=m_w.d_r.value;
				memRS->table[j].t_r.ready=1;
			}
		}
			for(int j=0;j<36;j++){

				if(FpRat->table[j].alias==w_c.d_r.id){
					FpRat->table[j].alias=j+101;
					FpRat->table[j].value=w_c.d_r.value;
					for(int i=0;i<instruction.size();i++){
					if(instruction[i].rd.id==m_w.d_r.id) instruction[i].rd.id=j+101;
					if(instruction[i].rs.id==m_w.d_r.id) instruction[i].rs.id=j+101;
					if(instruction[i].rt.id==m_w.d_r.id) instruction[i].rt.id=j+101;
					}

				
				}
			}
		for(int j=0;j<36;j++){

				if(IntRat->table[j].alias==m_w.d_r.id){
					IntRat->table[j].alias=j+1;
					IntRat->table[j].value=m_w.d_r.value;
					for(int i=0;i<instruction.size();i++){
					if(instruction[i].rd.id==m_w.d_r.id) instruction[i].rd.id=j+1;
					if(instruction[i].rs.id==m_w.d_r.id) instruction[i].rs.id=j+1;
					if(instruction[i].rt.id==m_w.d_r.id) instruction[i].rt.id=j+1;
					}
				}
			
		}
		w_c=m_w;
		instruction[m_w.icount].t_wb=clk;
		w_c.empty=0;
		m_w.empty=1;
	}
}

//commit

void commit()
{
	if(w_c.empty==0){
		w_c.empty=1;
		
		cmt_vector.push_back(w_c);
	}
	if(!cmt_vector.empty()){
		int min=0;
		for(int j=0;j<cmt_vector.size();j++){
			if(instruction[cmt_vector[min].icount].t_issue >instruction[cmt_vector[j].icount].t_issue)
				min=j;
		}
		if(cmt_vector[min].d_r.id==ROB->table[ROB->head].id){
			instruction[cmt_vector[min].icount].t_commit=clk;
			instruction[cmt_vector[min].icount].cmt=true;
			ROB->table[ROB->head].cmt_flag=1;
			ROB->head=(ROB->head+1)%num_ROB;
			ROB->size--;
			for(int j=0;j<num_iR;j++) IntArf->table[j].value=(int)IntRat->table[j].value;
			for(int j=0;j<num_fpR;j++)  FpArf->table[j].value=FpRat->table[j].value;
			vector<RS_entry>::iterator it_cmt_vector = cmt_vector.begin();
			cmt_vector.erase(it_cmt_vector+min);
		}
	}
}
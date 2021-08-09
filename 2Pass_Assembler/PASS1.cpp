#include "optab.h"
using namespace std;

int SymbolNUM = 0; //�ɺ� ���� �����ϱ� ���� ��������

struct CODE
{
	string label;
	string opcode;
	string operand;
};

struct SYMBOL
{
	int loc;       // ��ġ ����� LOC
	string symbol;
};

void DIV(const string buf, CODE &pass1);           //LABEL,OPCODE,OPERAND�� ������ �Լ�
int SYM_DIV(int locc, CODE pass1, SYMBOL *pass1S); //SYMTAB�� ����� ���� �Լ�
int LOC_Calculation(CODE pass1, int &loc);         //LOC���� ����ϴ� �Լ�

int main()
{
	int bSNUM;  // �ɺ� ���� ����Ҷ� ���ϱ� ���� ����
	CODE pass1; // ����ü pass1 ����
	string str;

	ifstream Input("input.txt", ios::in);           
	ofstream OutINTER("INTERMEDIATE.txt", ios::out);
	ofstream OutSYM("SYMTAB.txt", ios::out);

	SYMBOL SYM[20]; // �ɺ�����ü 20������

	int loc = 0;
	getline(Input, str);//input.txt ������ ù ���� �д´�.
	DIV(str, pass1);    //LABEL, OPCODE, OPERAND�� �и�

	if (pass1.opcode == "START")  //ù ������ OPCODE�� START�� ���
	{
		loc = strtol(&pass1.operand[0], NULL, 16); //OPERAND���� 16������ loc���� ���� -> 1000�� loc�� ���°�
		OutINTER.setf(ios::uppercase);             //16������ abcdf�� �빮�ڷ� �����ϱ� ���� �ڵ�
		// INTERMEDIATE File�� �� ���ο� ��ġ ������� ���� LABEL,OPCODE,OPERAND�� ����Ѵ�
		OutINTER << hex << loc << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl; 
		getline(Input, str); //���� ������ �а� �ٽ� LABEL,OPCODE,OPERAND�� �и��Ѵ�.
		DIV(str, pass1);
	}

	while (pass1.opcode != "END") // OPCODE�� END�� �� ���� �ݺ�
	{
		if (pass1.label == ".")   //�ּ��� ���
			if (pass1.opcode != " ")
				OutINTER << '\t' << pass1.label << '\t' << pass1.opcode << endl;
			else
				OutINTER << '\t' << pass1.label << endl;

		else  // �ּ��� �ƴ� ��� INTERMEDIATE File�� �� ���ο� ��ġ ������� ���� LABEL,OPCODE,OPERAND�� ����Ѵ�
		{
			OutINTER.setf(ios::uppercase);
			OutINTER << hex << loc << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl;
		}

		if (!LOC_Calculation(pass1, loc)) // ���� ó�� -> LOC_Calculation �� ���� �ƴҶ� ���� ��� �� ���α׷� ����
		{
			cout << "ERROR : Invalid Operation Code" << endl;
			cout << "Opcode -> " << pass1.opcode << endl;
			OutINTER << "ERROR : Invalid Operation Code" << endl;
			OutINTER << "Opcode -> " << pass1.opcode << endl;
			return 0;
		}

		getline(Input, str);  //���� ������ �о LABEL,OPCODE,OPERAND�� �и��Ѵ�. 
		DIV(str, pass1);


		bSNUM = SymbolNUM;    //���� ����� �ɺ����� ����

		if (!SYM_DIV(loc, pass1, SYM)) //���� ó�� -> SYM_DIV�� ���� �ƴҶ� ���� ��� �� ���α׷� ����
		{
			cout << "ERROR : Duplicate SYMBOL" << endl;
			cout << "Label -> " << pass1.label << endl;
			OutINTER << "ERROR : Duplicate SYMBOL" << endl;
			OutINTER << "Label -> " << pass1.label << endl;
			return 0;
		}

		if (bSNUM != SymbolNUM) {  //���ο� �ɺ��� �߰��Ǹ� Symbol table(SYMTAB����) �� 16������ Symbol�� ��ġ ����� �����Ѵ�.
			OutSYM.setf(ios::uppercase);
			OutSYM << SYM[SymbolNUM - 1].symbol << '\t' << hex << SYM[SymbolNUM - 1].loc << endl;	
		}
	}
	// ������� ���α׷��� �������� ����Ǿ��ٸ� PASS1 �Ϸ�
	OutINTER << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl;
	cout << "====================================================================" << endl;
	cout << "PASS1�� �Ϸ�Ǿ����ϴ�.\nINTERMEDIATE.txt ���ϰ� SYMTAB.txt ������ �����Ǿ����ϴ�." << endl;
	cout << "==================================================================== " << endl;

	Input.close();
	OutINTER.close();
	OutSYM.close();
}

void DIV(const string buf, CODE &pass1) // input.txt ���� �� �� ������ �о LABEL, OPCODE, OPERAND�� �и��ϴ� �Լ�     ex)  COPY	   START 	1000
{
	string temp;

	pass1.label = " ";
	pass1.opcode = " ";
	pass1.operand = " ";

	int i = 0;
	int size = buf.length();    // DIV �Ű����� buf�� ���� = �� ������ ����

	while(1)                    // LABEL�� �и��ϴ� �ݺ���
	{
		temp += buf[i];         // temp�� ���ڿ� buf�� ���� ���� �ϳ��� ����

		if (buf[i + 1] == '\t') // ���� ���ڰ� �����̸�(Ű���� Tab) pass1.label�� temp�� �־��ְ� temp�� �ʱ�ȭ �׸��� i�� ���� ������ ���������� ����Ű�� �ϰ� �ݺ����� �����Ѵ�..
		{
			pass1.label = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else if (buf[i + 1] == '\n') //���� ���ڰ� �ٹٲ��̸� pass1.label�� temp�� �־��ְ� temp�� �ʱ�ȭ �Ŀ� �� ���� ���������� �Լ��� �����Ѵ�.
		{
			pass1.label = temp;
			temp = "\0";
			return;
		}
		else if (buf[0] == '\t')     //buf�� ù ���ڰ� �����̸� (LABEL�� ���� ����) temp�� �ʱ�ȭ �� �ݺ����� �����Ѵ�.
		{
			temp = "\0";
			i++;
			break;
		}
		else if (i == size - 1)      //���� ����Ű�� ���ڰ� buf�� �������̸� pass1.label�� temp�� �־��ְ� �Լ��� �����Ѵ�. 
		{
			pass1.label = temp;
			return;
		}
		else i++;
	}

	while(1)                      //OPCODE�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (buf[i + 1] == '\t')  // ���� �ݺ����� ����ϰ� �۵�
		{
			pass1.opcode = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else if (buf[i + 1] == '\n')
		{
			pass1.opcode = temp;
			temp = "\0";
			return;
		}
		else if (i == size - 1)
		{
			pass1.opcode = temp;
			return;
		}
		else i++;
	}

	while(1)       //OPCODE�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (i == size - 1)     //���� ���ڵ��� ��� temp�� ���� �� pass1.operand�� �����Ѵ�.
		{
			pass1.operand = temp;
			break;
		}
		else i++;
	}
	return;
}

int SYM_DIV(int locc, CODE pass1, SYMBOL *pass1S)  //SYMTAB�� ����� ���� �Լ�
{
	pass1S->loc = 0;  //loc���� 0���� �ʱ�ȭ
	pass1S->symbol = "\0";
	if (pass1.label[0] != ' ') // pass1 ����ü ������ LABEL�� ������ �ƴϰ�
	{
		if (pass1.label[0] != '.') // �ּ��� �ƴϸ�
		{
			if (SymbolNUM == 0)    //SYMTAB�� �ƹ� ���� ���� ���
			{                      //LABEL�� ��ġ����� ���� �����Ѵ�.
				pass1S[SymbolNUM].symbol = pass1.label; 
				pass1S[SymbolNUM].loc = locc;
			}
			else  //SYMTAB�� ���� �� �ִ� ���
			{
				for (int i = 0; i<SymbolNUM; i++)         //���� SYMTAB�� �����ϴ� SYMBOL �� ��ŭ �ݺ�
					if (pass1.label == pass1S[i].symbol)  //�������� ���ؼ� �����ϸ� ����, 
						return 0;
				pass1S[SymbolNUM].symbol = pass1.label;   //�������� ������ LABEL�� ��ġ����� �� ����
				pass1S[SymbolNUM].loc = locc;
			}
			SymbolNUM++;                                  //SYMBOL �� ����
			return 1;
		}
	}
}


//Location counter���� ����� �� ��ɾ� �Ӹ� �ƴ϶� �����ҷ� ���̴� �κ�(assembler directive)���� ����Ͽ��� �Ѵ�.
int LOC_Calculation(CODE pass1, int &loc)  //��ġ ����� ���� ����ϴ� �Լ�
{
	int i;
	if (pass1.opcode == "BYTE")        //OPCODE�� BYTE�� ��� : ���� Ȥ�� 16���� ����� �����Ѵ�.
	{
		if (pass1.operand[0] == 'X')          //OPERAND�� ù ���ڰ� X�� ��� -> 16���� ��� ����
			loc += 1;
		else if (pass1.operand[0] == 'C')     //OPERAND�� ù ���ڰ� C�� ��� -> ���� ����
			loc += (pass1.operand.size() - 3);
	}
	else if (pass1.opcode == "WORD")   //OPCODE�� WORD�� ��� : �� ���� ���� ����� �����Ѵ�.
		loc += 3;
	else if (pass1.opcode == "RESW")   //OPCODE�� RESW�� ��� : �������� ����ŭ�� ������� �����Ѵ�.
		loc += (atoi(pass1.operand.c_str()) * 3);
	else if (pass1.opcode == "RESB")   //OPCODE�� RESB�� ��� : �������� ����ŭ�� ����Ʈ���� �����Ѵ�.
		loc += (atoi(pass1.operand.c_str()));
	else  //����� �����ڰ� �ƴѰ��
	{
		for (i = 0; i < 25; i++) {            // OPTAB�� ������ŭ �ݺ� 		
			string c = OPTAB[i].opcode;
			if (pass1.opcode.compare(c) == 0) //OPCODE�� OPTAB�� �����ϴ� ���
			{
				loc += 3;
				break;
			}
		}
		if (i == 25)
			if (pass1.label[0] != '.') {
				cout << "Ž�� ���� "<< pass1.opcode << endl;
				return 0;  //Ž�� ����
			}
	}
	return 1;  //Ž�� ����
}

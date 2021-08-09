#include "optab.h"
using namespace std;

int SYMNUM = 0;

struct CODE
{
	string loc;
	string label;
	string opcode;
	string operand;
	string object;
};

struct SYMBOL
{
	string loc;
	string symbol;
};

void DIV(const string buf, CODE &pass2, SYMBOL *SYM); //loc,label,opcode,operand,object�� ������ �Լ�
void operand_DIV(CODE &pass2, SYMBOL *SYM);			 //�ε��� �������� ����� ����ϱ� ���� �Լ�
void SYM_DIV(const string buf, SYMBOL *SYM);		 //�Է¹��� SYMTAB�� LOC�� SYMBOL�� �и��ϴ� �Լ�
void fix(CODE &pass2);								 //16���� �ڸ����� ä���ִ� �Լ�
void ERROR(ofstream &OutLIST, CODE &pass2);		     //���� �޽��� ��� �Լ�
void output(ofstream &OutLIST, CODE &pass2, int i);  //List File ��� �Լ�

ifstream InINTER("INTERMEDIATE.txt", ios::in);      //INTERMEDIATE File �б�
ifstream InSYM("SYMTAB.txt", ios::in);              //SYMTAB File �б�
ofstream OutOBJECT("OBJECT Program.txt", ios::out); //Object File ���� 
ofstream OutLIST("LIST File.txt", ios::out);        //LIST File ����

int main()
{
	int i = 5;       //LIST FILE�� ����ϴ� ���� -> 5���ͽ���
	int RES_cnt = 0;
	int TR_len = 0;  //text Record�� ����
	int HR_len = 0;  //Header Record�� ����
	string start;    //object file�� End Record�� ����� loc��
	string TR_loc;   //Text Record�� ���Ե� object code�� ���� �ּ�
	string str, temp;
	CODE pass2;

	SYMBOL SYM[20];
	while (!InSYM.eof())			 //SYMTAB.txt ������ ������ �ݺ�
	{
		getline(InSYM, str);         //SYMTAB.txt������ �� ������ �о 
		SYM_DIV(str, SYM + SYMNUM);  //SYMBOL�� LOC�� �и��ϰ� SYM ����ü ������ �����Ѵ�.
		SYMNUM++;
	}
	getline(InINTER, str);           //INTERMEDIATE ������ ù ������ �д´�.
	
	DIV(str, pass2, SYM);
	/*if (!DIV(str, pass2, SYM))
	{
		ERROR(OutLIST, pass2);
		return 0;
	}*/
	fix(pass2);
	HR_len += (pass2.object.length() / 2);

	if (pass2.opcode == "START")
	{
		start = pass2.loc;  //������Ʈ �ڵ��� End Record�� ����� loc��
		// LIST ������ ù ���� ����Ѵ�.
		OutLIST << "Line" << '\t' << "Loc" << '\t' << "Label" << '\t' << "Opcode" << '\t' << "Operand" << "\t\t" << "Object Code" << endl << endl;
		// OBJECT FILE�� Header Record�� ����Ѵ�.
		// ���� 6��ŭ���� �ϰ� LOC���� ����ϰ� ���� �ڸ��� 0���� ä���.
		OutOBJECT << "H" << pass2.label << ' '; OutOBJECT.width(6);   OutOBJECT.fill('0'); OutOBJECT << pass2.loc << "      " << endl;
		output(OutLIST, pass2, i); // LIST FILE�� pass2�� ������ ����Ѵ�.
		i += 5;                    // INTERMEDIATE ���� 5 ����
		getline(InINTER, str);     // INTERMEDIATE ������ ���� ������ �д´�
		DIV(str, pass2, SYM);
		/*if (!DIV(str, pass2, SYM))
		{
			ERROR(OutLIST, pass2);
			return 0;
		}*/
		fix(pass2);
		HR_len += (pass2.object.length() / 2);
	}

	while (pass2.opcode != "END") // �о���� ������ OPCODE�� END�� �� ���� �ݺ� 
	{
		if (pass2.label == ".")   // �ּ��� ��� �׳�
			OutLIST << i << "\t\t" << pass2.label << '\t' << pass2.opcode << endl; //
		else                      // �ּ��� �ƴ� ���
		{
			if (pass2.operand[0] == 'X') // 16���� ����� ���
				OutLIST << i << '\t' << pass2.loc << '\t' << pass2.label << '\t' << pass2.opcode << '\t' << pass2.operand << "\t\t" << pass2.object << endl;
			else
				output(OutLIST, pass2, i);

			if (TR_len == 0)                 //Text Record�� ���������
				TR_loc = pass2.loc;          //Record�� ���Ե� object code�� ���� �ּҸ� ����

			if (pass2.object != "")          //Object File�� ����� ���� �ʿ��� �����͸� ����
			{
				//RESW, RESB�� �޸��� ���ุ�� �����ϰ� �������� ������ ����.
				//���� object code�� ����, �̴� Text Record�� ���̸� ������Ű�� �ʴ´�.
				if (pass2.opcode == "RESW" || pass2.opcode == "RESB")
					RES_cnt++;
				else if (pass2.opcode == "BYTE") {
					TR_len += pass2.object.length();
					temp += pass2.object;
					RES_cnt++;
				}
				else {
					TR_len += pass2.object.length(); //���� ������ object code�� ���̸�ŭ ����
					temp += pass2.object;
					RES_cnt++;
				}
			}

			if (RES_cnt > 9)  //Text Record�� ����� Object code�� 10���� �Ǹ�
			{
				OutOBJECT.setf(ios::uppercase);
				OutOBJECT << "T00" << TR_loc; OutOBJECT.width(2); OutOBJECT.fill('0'); OutOBJECT << hex << TR_len / 2 << temp << endl;				
				TR_len = 0;
				temp = "\0";
				RES_cnt = 0;
			}
		}
		i += 5;
		getline(InINTER, str);
		DIV(str, pass2, SYM);
		fix(pass2);

		if (pass2.opcode == "RESB")                      //OPCODE�� RESB�� ��� : �������� ����ŭ�� ����Ʈ���� �����Ѵ�.
			HR_len += atoi(pass2.operand.c_str());
		else if (pass2.opcode == "RESW")                 //OPCODE�� RESW�� ��� : �������� ����ŭ�� ������� �����Ѵ�.
			HR_len += (atoi(pass2.operand.c_str()) * 3);
		else                                             //�� ���� ���
			HR_len += (pass2.object.length() / 2);       
	}

	output(OutLIST, pass2, i);
	if (TR_len != 0) {  //����ؾ��� Text Record�� �����ִٸ� ������ Text Record �ۼ�
		OutOBJECT.setf(ios::uppercase);
		OutOBJECT << "T00" << TR_loc; OutOBJECT.width(2); OutOBJECT.fill('0'); OutOBJECT << hex << TR_len / 2 << temp << endl;
	}
	//End Record �ۼ�
	OutOBJECT << "E"; OutOBJECT.width(6); OutOBJECT.fill('0'); OutOBJECT << start << endl;
	//Head Record�� �� 14-19  (����Ʈ�� ǥ�õ� ���� ���α׷��� ����)�� HR_len���� ��ü�Ѵ�.
	OutOBJECT.width(6); OutOBJECT.fill('0'); OutOBJECT.seekp(12, ios::beg); OutOBJECT << HR_len << endl;
	// ������� ���α׷��� �������� ����Ǿ��ٸ� PASS2 �Ϸ�
	cout << "====================================================================" << endl;
	cout << "PASS2�� �Ϸ�Ǿ����ϴ�.\nLIST File.txt ���ϰ� OBJECT Program.txt ������ �����Ǿ����ϴ�." << endl;
	cout << "==================================================================== " << endl;


	InINTER.close();
	InSYM.close();
	OutLIST.close();
	OutOBJECT.close();
}

void DIV(const string buf, CODE &pass2, SYMBOL *SYM)  //�Է¹��� ������ loc,label,opcode,operand�� ������
{                                                    //SYMTAB�� �����Ͽ� object code�� ����
	string temp;									 //PASS1.cpp ������ DIV�� �����ϰ� �۵��Ѵ�.
	char buff[64];

	pass2.loc = " ";
	pass2.label = " ";
	pass2.opcode = " ";
	pass2.operand = " ";
	pass2.object = " ";

	int i = 0;
	int size = buf.length();

	while(1)  // LOC�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (buf[i + 1] == '\t')
		{
			if (buf[0] == '\t')
			{
				pass2.loc = " ";
				temp = "\0";
				i++;
				break;
			}
			pass2.loc = temp;
			temp = "\0";
			i+=2;
			break;

		}
		else if (buf[0] == '\t')
		{
			pass2.loc = " ";
			temp = "\0";
			i++;
			break;
		}
		else i++;
	}

	while(1) // LABEL�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (buf[i + 1] == '\t')
		{
			pass2.label = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else if (buf[i + 1] == '\n')
		{
			pass2.label = temp;
			temp = "\0";
			return;
		}
		else if (buf[i] == '\t')
		{
			temp = "\0";
			i++;
			break;
		}
		else if (i == size - 1)
		{
			pass2.label = temp;
			return;
		}
		else i++;
	}

	while (1)  // OPCODE�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (buf[i + 1] == '\t')
		{
			pass2.opcode = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else if (buf[i + 1] == '\n')
		{
			pass2.opcode = temp;
			temp = "\0";
			return;
		}
		else if (i == size - 1)
		{
			pass2.opcode = temp;
			return;
		}
		else i++;
	}

	while(1)  // OPERAND�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (i == size - 1)
		{
			pass2.operand = temp;
			break;
		}
		else i++;
	}

	//���α׷����� ������ LOC ���� �������� �� ���α׷����� ASCII�� ���� ���� binery�� ��ȯ�Ͽ� �ش� ��ҿ� �ִ� ���̴�. 
	if (pass2.opcode == "WORD" || pass2.opcode == "RESB" || pass2.opcode == "RESW" || pass2.opcode == "BYTE")  //OPCODE�� assembler directive�ΰ��
	{
		pass2.object = "\0";
		if (pass2.opcode == "BYTE")       //OPCODE�� BYTE�� ��� : ���� Ȥ�� 16���� ����� �����Ѵ�.
		{
			if (pass2.operand[0] == 'C')  //OPERAND�� ù ���ڰ� C�� ��� -> ���� ����
			{
				int num = pass2.operand.length();
				for (int i = 2; i<num - 1; i++)
				{
					int j = pass2.operand[i];
					sprintf_s(buff, "%X", j);
					pass2.object += buff; //object code�� OPERAND�� 16����
				}
				return;
			}
			else if (pass2.operand[0] == 'X')  //OPERAND�� ù ���ڰ� X�� ��� -> 16���� ��� ����
			{
				int num = pass2.operand.length();
				for (int i = 2; i<num - 1; i++)
					pass2.object += pass2.operand[i];
				return;
			}
		}
		else if (pass2.opcode == "WORD")  //OPCODE�� WORD�� ��� : �� ���� ���� ����� �����Ѵ�.
		{
			int j = atoi(pass2.operand.c_str());
			sprintf_s(buff, "%X", j);
			pass2.object = buff;
			return;
		}
		else  //RESB, RESW�� �޸𸮸� �Ҵ��ϰ� �����ڵ�� ��������� ����
		{
			pass2.object = " ";  //�׷��Ƿ� ����
			return;
		}
	}
	else  //OPCODE�� assembler directive�� �ƴ� ���
	{
		int j;
		for (i = 0; i<25; i++)
		{
			if (pass2.opcode == OPTAB[i].opcode) //OPCODE�� OPTAB�� �����Ѵٸ�
			{
				sprintf_s(buff, "%X", OPTAB[i].code); //object code�� OPCODE�� code�� 16������ ����
				pass2.object = buff;  
				if (pass2.operand != " ")                                  //OPERAND�� ������ �ƴϰ�
					if (pass2.operand[pass2.operand.length() - 2] == ',') {  //�ε��� �������Ͷ��
						operand_DIV(pass2, SYM);                           //object code�� ����ϱ� ���� �Լ� ȣ��
						return;
					}

				for (j = 0; j<SYMNUM; j++)  //�ε��� ����� �ƴ϶�� SYMTAB�� Ž��
				{
					if (pass2.operand == SYM[j].symbol)  //OPERAND�� SYMTAB�� �����Ѵٸ�	
					{
						pass2.object += SYM[j].loc;      //object code�� SYMBOL�� code�� ����
						break;
					}
					if (j == 15)
						if (pass2.operand == " ")
							pass2.object += "0000";
						else if (pass2.loc[0] != '.')
						{
							if (pass2.operand != " ")
								ERROR(OutLIST, pass2);
						}
				}
			}
		}
		return;
	}
}

void operand_DIV(CODE &pass2, SYMBOL *SYM)  //�ε��� �������� ����� ����ϱ� ���� �Լ�
{
	string temp = "\0";
	char buff[64];
	int i = 0;
	int size = pass2.operand.length();

	if (pass2.operand[size - 2] == ',')
	{
		while (pass2.operand[i] != ',') // ','�� �������� ������ ���� �ݺ���
		{
			temp += pass2.operand[i];
			i++;
		}

		for (int j = 0; j<SYMNUM; j++)
			if (temp == SYM[j].symbol)  //SYMTAB�� ��ġ�ϴ� SYMBOL�� �ִ°��
			{
				i = strtol(&(SYM + j)->loc[0], NULL, 16);  //SYMBOL�� �����Ǵ� LOC���� 16������ ������ ����
				i += 32768;								   //�ε��� �������� ����̹Ƿ� 2^16���ڸ��� 1�� �Ǵ� 10���� 32768�� ������
				                                           //OPCODE(8) X(1) ADDRESS(15) <- X �κ��� ��Ʈ�� 1�� set �Ѵ�.
				sprintf_s(buff, "%X", i);				   //���� i�� ���� 16������ ������ Object�� ����
				pass2.object += buff;
				return;
			}
	}
}

void SYM_DIV(const string buf, SYMBOL *SYM) // SYMTAB.txt ���� �� �� ������ �о LOC, SYMBOL�� �и��ϴ� �Լ�     ex) CLOOP	1003
{
	string temp;

	SYM->loc = " ";
	SYM->symbol = " ";

	int i = 0;
	int size = buf.length();     // �Ű����� buf�� ���� = �� ������ ����

	if (size == 0)
		return;

	while (1)                     // SYMBOL�� �и��ϴ� �ݺ���
	{
		temp += buf[i];          // temp�� ���ڿ� buf�� ���� ���� �ϳ��� ����

		if (buf[i + 1] == '\t')  // ���� ���ڰ� �����̸�(Ű���� Tab) SYM->symbol�� temp�� �����ϰ� temp�� �ʱ�ȭ �׸��� i�� ���� ������ ���������� ����Ű�� �ϰ� �ݺ����� �����Ѵ�.
		{
			SYM->symbol = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else i++;
	}


	while (1)                     // LOC�� �и��ϴ� �ݺ���
	{
		temp += buf[i];

		if (i == size - 1)       // ���� ����Ű�� ���ڰ� buf�� �������̸� pass2.label�� temp�� �����Ѵ�.
		{
			SYM->loc = temp;
			break;
		}
		else i++;
	}
}

void fix(CODE &pass2)  //16���� �ڸ����� ä���ִ� �Լ�(6�ڸ����ȵǸ� �տ� 0�� ä����)
{
	if (pass2.object != " ")
		if (pass2.opcode != "BYTE")
			if (pass2.object.length()<6)
				while (pass2.object.length()<6)
					pass2.object.insert(0, "0");
}


void ERROR(ofstream &OutLIST, CODE &pass2)  //���� �޽��� ���
{
	OutLIST << "ERROR : Undefined Symbol" << endl;
	OutLIST << "Operand : " << pass2.operand << endl;
	cout << "ERROR : Undefined Symbol" << endl;
	cout << "Operand : " << pass2.operand << endl;
}

void output(ofstream &OutLIST, CODE &pass2, int i)  //List File ��� �Լ�
{
	if (pass2.object != " ") {
		if (pass2.operand.compare("BUFFER,X") == 0)
			OutLIST << i << '\t' << pass2.loc << '\t' << pass2.label << '\t' << pass2.opcode << '\t' << pass2.operand << "\t" << pass2.object << endl;
		else
			OutLIST << i << '\t' << pass2.loc << '\t' << pass2.label << '\t' << pass2.opcode << '\t' << pass2.operand << "\t\t" << pass2.object << endl;
	}
	else
		OutLIST << i << '\t' << pass2.loc << '\t' << pass2.label << '\t' << pass2.opcode << '\t' << pass2.operand << endl;
}
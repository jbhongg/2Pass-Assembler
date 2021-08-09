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

void DIV(const string buf, CODE &pass2, SYMBOL *SYM); //loc,label,opcode,operand,object로 나누는 함수
void operand_DIV(CODE &pass2, SYMBOL *SYM);			 //인덱스 레지스터 방식을 계산하기 위한 함수
void SYM_DIV(const string buf, SYMBOL *SYM);		 //입력받은 SYMTAB을 LOC와 SYMBOL로 분리하는 함수
void fix(CODE &pass2);								 //16진수 자리수를 채워주는 함수
void ERROR(ofstream &OutLIST, CODE &pass2);		     //에러 메시지 출력 함수
void output(ofstream &OutLIST, CODE &pass2, int i);  //List File 출력 함수

ifstream InINTER("INTERMEDIATE.txt", ios::in);      //INTERMEDIATE File 읽기
ifstream InSYM("SYMTAB.txt", ios::in);              //SYMTAB File 읽기
ofstream OutOBJECT("OBJECT Program.txt", ios::out); //Object File 생성 
ofstream OutLIST("LIST File.txt", ios::out);        //LIST File 생성

int main()
{
	int i = 5;       //LIST FILE에 출력하는 라인 -> 5부터시작
	int RES_cnt = 0;
	int TR_len = 0;  //text Record의 길이
	int HR_len = 0;  //Header Record의 길이
	string start;    //object file의 End Record에 사용할 loc값
	string TR_loc;   //Text Record에 포함될 object code의 시작 주소
	string str, temp;
	CODE pass2;

	SYMBOL SYM[20];
	while (!InSYM.eof())			 //SYMTAB.txt 파일의 끝까지 반복
	{
		getline(InSYM, str);         //SYMTAB.txt파일의 한 라인을 읽어서 
		SYM_DIV(str, SYM + SYMNUM);  //SYMBOL과 LOC를 분리하고 SYM 구조체 변수에 저장한다.
		SYMNUM++;
	}
	getline(InINTER, str);           //INTERMEDIATE 파일의 첫 라인을 읽는다.
	
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
		start = pass2.loc;  //오브젝트 코드의 End Record에 사용할 loc값
		// LIST 파일의 첫 줄을 출력한다.
		OutLIST << "Line" << '\t' << "Loc" << '\t' << "Label" << '\t' << "Opcode" << '\t' << "Operand" << "\t\t" << "Object Code" << endl << endl;
		// OBJECT FILE의 Header Record를 출력한다.
		// 폭을 6만큼으로 하고 LOC값을 출력하고 남는 자리를 0으로 채운다.
		OutOBJECT << "H" << pass2.label << ' '; OutOBJECT.width(6);   OutOBJECT.fill('0'); OutOBJECT << pass2.loc << "      " << endl;
		output(OutLIST, pass2, i); // LIST FILE에 pass2의 정보를 출력한다.
		i += 5;                    // INTERMEDIATE 라인 5 증가
		getline(InINTER, str);     // INTERMEDIATE 파일의 다음 라인을 읽는다
		DIV(str, pass2, SYM);
		/*if (!DIV(str, pass2, SYM))
		{
			ERROR(OutLIST, pass2);
			return 0;
		}*/
		fix(pass2);
		HR_len += (pass2.object.length() / 2);
	}

	while (pass2.opcode != "END") // 읽어오는 라인의 OPCODE가 END일 때 까지 반복 
	{
		if (pass2.label == ".")   // 주석일 경우 그냥
			OutLIST << i << "\t\t" << pass2.label << '\t' << pass2.opcode << endl; //
		else                      // 주석이 아닐 경우
		{
			if (pass2.operand[0] == 'X') // 16진수 상수일 경우
				OutLIST << i << '\t' << pass2.loc << '\t' << pass2.label << '\t' << pass2.opcode << '\t' << pass2.operand << "\t\t" << pass2.object << endl;
			else
				output(OutLIST, pass2, i);

			if (TR_len == 0)                 //Text Record가 비어있으면
				TR_loc = pass2.loc;          //Record에 포함될 object code의 시작 주소를 저장

			if (pass2.object != "")          //Object File을 만들기 위해 필요한 데이터를 저장
			{
				//RESW, RESB는 메모리의 예약만을 지시하고 데이터의 생성은 없다.
				//따라서 object code가 없고, 이는 Text Record의 길이를 증가시키지 않는다.
				if (pass2.opcode == "RESW" || pass2.opcode == "RESB")
					RES_cnt++;
				else if (pass2.opcode == "BYTE") {
					TR_len += pass2.object.length();
					temp += pass2.object;
					RES_cnt++;
				}
				else {
					TR_len += pass2.object.length(); //현재 라인의 object code의 길이만큼 증가
					temp += pass2.object;
					RES_cnt++;
				}
			}

			if (RES_cnt > 9)  //Text Record에 저장된 Object code가 10개가 되면
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

		if (pass2.opcode == "RESB")                      //OPCODE가 RESB인 경우 : 지정해준 수만큼의 바이트들을 예약한다.
			HR_len += atoi(pass2.operand.c_str());
		else if (pass2.opcode == "RESW")                 //OPCODE가 RESW인 경우 : 지정해준 수만큼의 워드들을 예약한다.
			HR_len += (atoi(pass2.operand.c_str()) * 3);
		else                                             //그 외의 경우
			HR_len += (pass2.object.length() / 2);       
	}

	output(OutLIST, pass2, i);
	if (TR_len != 0) {  //출력해야할 Text Record가 남아있다면 마지막 Text Record 작성
		OutOBJECT.setf(ios::uppercase);
		OutOBJECT << "T00" << TR_loc; OutOBJECT.width(2); OutOBJECT.fill('0'); OutOBJECT << hex << TR_len / 2 << temp << endl;
	}
	//End Record 작성
	OutOBJECT << "E"; OutOBJECT.width(6); OutOBJECT.fill('0'); OutOBJECT << start << endl;
	//Head Record의 열 14-19  (바이트로 표시된 목적 프로그램의 길이)를 HR_len으로 교체한다.
	OutOBJECT.width(6); OutOBJECT.fill('0'); OutOBJECT.seekp(12, ios::beg); OutOBJECT << HR_len << endl;
	// 여기까지 프로그램이 막힘없이 진행되었다면 PASS2 완료
	cout << "====================================================================" << endl;
	cout << "PASS2가 완료되었습니다.\nLIST File.txt 파일과 OBJECT Program.txt 파일이 생성되었습니다." << endl;
	cout << "==================================================================== " << endl;


	InINTER.close();
	InSYM.close();
	OutLIST.close();
	OutOBJECT.close();
}

void DIV(const string buf, CODE &pass2, SYMBOL *SYM)  //입력받은 라인을 loc,label,opcode,operand로 나누고
{                                                    //SYMTAB을 참고하여 object code를 생성
	string temp;									 //PASS1.cpp 파일의 DIV와 동일하게 작동한다.
	char buff[64];

	pass2.loc = " ";
	pass2.label = " ";
	pass2.opcode = " ";
	pass2.operand = " ";
	pass2.object = " ";

	int i = 0;
	int size = buf.length();

	while(1)  // LOC를 분리하는 반복문
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

	while(1) // LABEL을 분리하는 반복문
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

	while (1)  // OPCODE를 분리하는 반복문
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

	while(1)  // OPERAND를 분리하는 반복문
	{
		temp += buf[i];

		if (i == size - 1)
		{
			pass2.operand = temp;
			break;
		}
		else i++;
	}

	//프로그램에서 변수에 LOC 값을 선언했을 때 프로그램에서 ASCII로 읽은 값을 binery로 변환하여 해당 장소에 넣는 것이다. 
	if (pass2.opcode == "WORD" || pass2.opcode == "RESB" || pass2.opcode == "RESW" || pass2.opcode == "BYTE")  //OPCODE가 assembler directive인경우
	{
		pass2.object = "\0";
		if (pass2.opcode == "BYTE")       //OPCODE가 BYTE인 경우 : 문자 혹은 16진수 상수를 생성한다.
		{
			if (pass2.operand[0] == 'C')  //OPERAND의 첫 문자가 C일 경우 -> 문자 생성
			{
				int num = pass2.operand.length();
				for (int i = 2; i<num - 1; i++)
				{
					int j = pass2.operand[i];
					sprintf_s(buff, "%X", j);
					pass2.object += buff; //object code는 OPERAND의 16진수
				}
				return;
			}
			else if (pass2.operand[0] == 'X')  //OPERAND의 첫 문자가 X일 경우 -> 16진수 상수 생성
			{
				int num = pass2.operand.length();
				for (int i = 2; i<num - 1; i++)
					pass2.object += pass2.operand[i];
				return;
			}
		}
		else if (pass2.opcode == "WORD")  //OPCODE가 WORD인 경우 : 한 워드 정수 상수를 생성한다.
		{
			int j = atoi(pass2.operand.c_str());
			sprintf_s(buff, "%X", j);
			pass2.object = buff;
			return;
		}
		else  //RESB, RESW는 메모리만 할당하고 목적코드는 만들어지지 않음
		{
			pass2.object = " ";  //그러므로 공백
			return;
		}
	}
	else  //OPCODE가 assembler directive가 아닌 경우
	{
		int j;
		for (i = 0; i<25; i++)
		{
			if (pass2.opcode == OPTAB[i].opcode) //OPCODE가 OPTAB에 존재한다면
			{
				sprintf_s(buff, "%X", OPTAB[i].code); //object code에 OPCODE의 code를 16진수로 저장
				pass2.object = buff;  
				if (pass2.operand != " ")                                  //OPERAND가 공백이 아니고
					if (pass2.operand[pass2.operand.length() - 2] == ',') {  //인덱스 레지스터라면
						operand_DIV(pass2, SYM);                           //object code를 계산하기 위해 함수 호출
						return;
					}

				for (j = 0; j<SYMNUM; j++)  //인덱스 방식이 아니라면 SYMTAB을 탐색
				{
					if (pass2.operand == SYM[j].symbol)  //OPERAND가 SYMTAB에 존재한다면	
					{
						pass2.object += SYM[j].loc;      //object code에 SYMBOL의 code를 저장
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

void operand_DIV(CODE &pass2, SYMBOL *SYM)  //인덱스 레지스터 방식을 계산하기 위한 함수
{
	string temp = "\0";
	char buff[64];
	int i = 0;
	int size = pass2.operand.length();

	if (pass2.operand[size - 2] == ',')
	{
		while (pass2.operand[i] != ',') // ','를 기준으로 나누기 위한 반복문
		{
			temp += pass2.operand[i];
			i++;
		}

		for (int j = 0; j<SYMNUM; j++)
			if (temp == SYM[j].symbol)  //SYMTAB에 일치하는 SYMBOL이 있는경우
			{
				i = strtol(&(SYM + j)->loc[0], NULL, 16);  //SYMBOL에 대응되는 LOC값을 16진수로 변수에 저장
				i += 32768;								   //인덱스 레지스터 방식이므로 2^16승자리에 1이 되는 10진수 32768를 더해줌
				                                           //OPCODE(8) X(1) ADDRESS(15) <- X 부분의 비트를 1로 set 한다.
				sprintf_s(buff, "%X", i);				   //계산된 i의 값을 16진수의 값으로 Object에 저장
				pass2.object += buff;
				return;
			}
	}
}

void SYM_DIV(const string buf, SYMBOL *SYM) // SYMTAB.txt 파일 의 한 라인을 읽어서 LOC, SYMBOL을 분리하는 함수     ex) CLOOP	1003
{
	string temp;

	SYM->loc = " ";
	SYM->symbol = " ";

	int i = 0;
	int size = buf.length();     // 매개변수 buf의 길이 = 한 라인의 길이

	if (size == 0)
		return;

	while (1)                     // SYMBOL을 분리하는 반복문
	{
		temp += buf[i];          // temp에 문자열 buf의 현재 문자 하나를 복사

		if (buf[i + 1] == '\t')  // 다음 문자가 공백이면(키보드 Tab) SYM->symbol에 temp를 저장하고 temp를 초기화 그리고 i를 현재 문자의 다음다음을 가르키게 하고 반복문을 종료한다.
		{
			SYM->symbol = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else i++;
	}


	while (1)                     // LOC를 분리하는 반복문
	{
		temp += buf[i];

		if (i == size - 1)       // 현재 가리키는 문자가 buf의 마지막이면 pass2.label에 temp를 저장한다.
		{
			SYM->loc = temp;
			break;
		}
		else i++;
	}
}

void fix(CODE &pass2)  //16진수 자리수를 채워주는 함수(6자리가안되면 앞에 0을 채워줌)
{
	if (pass2.object != " ")
		if (pass2.opcode != "BYTE")
			if (pass2.object.length()<6)
				while (pass2.object.length()<6)
					pass2.object.insert(0, "0");
}


void ERROR(ofstream &OutLIST, CODE &pass2)  //에러 메시지 출력
{
	OutLIST << "ERROR : Undefined Symbol" << endl;
	OutLIST << "Operand : " << pass2.operand << endl;
	cout << "ERROR : Undefined Symbol" << endl;
	cout << "Operand : " << pass2.operand << endl;
}

void output(ofstream &OutLIST, CODE &pass2, int i)  //List File 출력 함수
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
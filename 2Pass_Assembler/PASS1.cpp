#include "optab.h"
using namespace std;

int SymbolNUM = 0; //심볼 수를 저장하기 위한 전역변수

struct CODE
{
	string label;
	string opcode;
	string operand;
};

struct SYMBOL
{
	int loc;       // 위치 계수기 LOC
	string symbol;
};

void DIV(const string buf, CODE &pass1);           //LABEL,OPCODE,OPERAND로 나누는 함수
int SYM_DIV(int locc, CODE pass1, SYMBOL *pass1S); //SYMTAB을 만들기 위한 함수
int LOC_Calculation(CODE pass1, int &loc);         //LOC값을 계산하는 함수

int main()
{
	int bSNUM;  // 심볼 수를 계산할때 비교하기 위한 변수
	CODE pass1; // 구조체 pass1 선언
	string str;

	ifstream Input("input.txt", ios::in);           
	ofstream OutINTER("INTERMEDIATE.txt", ios::out);
	ofstream OutSYM("SYMTAB.txt", ios::out);

	SYMBOL SYM[20]; // 심볼구조체 20개선언

	int loc = 0;
	getline(Input, str);//input.txt 파일의 첫 줄을 읽는다.
	DIV(str, pass1);    //LABEL, OPCODE, OPERAND를 분리

	if (pass1.opcode == "START")  //첫 라인의 OPCODE가 START인 경우
	{
		loc = strtol(&pass1.operand[0], NULL, 16); //OPERAND값을 16진수로 loc값에 저장 -> 1000이 loc에 들어가는것
		OutINTER.setf(ios::uppercase);             //16진수의 abcdf를 대문자로 저장하기 위한 코드
		// INTERMEDIATE File의 한 라인에 위치 계수기의 값과 LABEL,OPCODE,OPERAND를 출력한다
		OutINTER << hex << loc << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl; 
		getline(Input, str); //다음 라인을 읽고 다시 LABEL,OPCODE,OPERAND를 분리한다.
		DIV(str, pass1);
	}

	while (pass1.opcode != "END") // OPCODE가 END일 때 까지 반복
	{
		if (pass1.label == ".")   //주석일 경우
			if (pass1.opcode != " ")
				OutINTER << '\t' << pass1.label << '\t' << pass1.opcode << endl;
			else
				OutINTER << '\t' << pass1.label << endl;

		else  // 주석이 아닐 경우 INTERMEDIATE File의 한 라인에 위치 계수기의 값과 LABEL,OPCODE,OPERAND를 출력한다
		{
			OutINTER.setf(ios::uppercase);
			OutINTER << hex << loc << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl;
		}

		if (!LOC_Calculation(pass1, loc)) // 예외 처리 -> LOC_Calculation 이 참이 아닐때 에러 출력 후 프로그램 종료
		{
			cout << "ERROR : Invalid Operation Code" << endl;
			cout << "Opcode -> " << pass1.opcode << endl;
			OutINTER << "ERROR : Invalid Operation Code" << endl;
			OutINTER << "Opcode -> " << pass1.opcode << endl;
			return 0;
		}

		getline(Input, str);  //다음 라인을 읽어서 LABEL,OPCODE,OPERAND를 분리한다. 
		DIV(str, pass1);


		bSNUM = SymbolNUM;    //현재 저장된 심볼수를 저장

		if (!SYM_DIV(loc, pass1, SYM)) //예외 처리 -> SYM_DIV가 참이 아닐때 에러 출력 후 프로그램 종료
		{
			cout << "ERROR : Duplicate SYMBOL" << endl;
			cout << "Label -> " << pass1.label << endl;
			OutINTER << "ERROR : Duplicate SYMBOL" << endl;
			OutINTER << "Label -> " << pass1.label << endl;
			return 0;
		}

		if (bSNUM != SymbolNUM) {  //새로운 심볼이 추가되면 Symbol table(SYMTAB파일) 에 16진수로 Symbol과 위치 계수를 저장한다.
			OutSYM.setf(ios::uppercase);
			OutSYM << SYM[SymbolNUM - 1].symbol << '\t' << hex << SYM[SymbolNUM - 1].loc << endl;	
		}
	}
	// 여기까지 프로그램이 막힘없이 진행되었다면 PASS1 완료
	OutINTER << '\t' << pass1.label << '\t' << pass1.opcode << '\t' << pass1.operand << endl;
	cout << "====================================================================" << endl;
	cout << "PASS1이 완료되었습니다.\nINTERMEDIATE.txt 파일과 SYMTAB.txt 파일이 생성되었습니다." << endl;
	cout << "==================================================================== " << endl;

	Input.close();
	OutINTER.close();
	OutSYM.close();
}

void DIV(const string buf, CODE &pass1) // input.txt 파일 의 한 라인을 읽어서 LABEL, OPCODE, OPERAND를 분리하는 함수     ex)  COPY	   START 	1000
{
	string temp;

	pass1.label = " ";
	pass1.opcode = " ";
	pass1.operand = " ";

	int i = 0;
	int size = buf.length();    // DIV 매개변수 buf의 길이 = 한 라인의 길이

	while(1)                    // LABEL을 분리하는 반복문
	{
		temp += buf[i];         // temp에 문자열 buf의 현재 문자 하나를 복사

		if (buf[i + 1] == '\t') // 다음 문자가 공백이면(키보드 Tab) pass1.label에 temp를 넣어주고 temp를 초기화 그리고 i를 현재 문자의 다음다음을 가르키게 하고 반복문을 종료한다..
		{
			pass1.label = temp;
			temp = "\0";
			i += 2;
			break;
		}
		else if (buf[i + 1] == '\n') //다음 문자가 줄바꿈이면 pass1.label에 temp를 넣어주고 temp를 초기화 후에 한 줄이 끝났음으로 함수를 종료한다.
		{
			pass1.label = temp;
			temp = "\0";
			return;
		}
		else if (buf[0] == '\t')     //buf의 첫 문자가 공백이면 (LABEL이 없는 라인) temp를 초기화 후 반복문을 종료한다.
		{
			temp = "\0";
			i++;
			break;
		}
		else if (i == size - 1)      //현재 가리키는 문자가 buf의 마지막이면 pass1.label에 temp를 넣어주고 함수를 종료한다. 
		{
			pass1.label = temp;
			return;
		}
		else i++;
	}

	while(1)                      //OPCODE를 분리하는 반복문
	{
		temp += buf[i];

		if (buf[i + 1] == '\t')  // 위의 반복문과 비슷하게 작동
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

	while(1)       //OPCODE를 분리하는 반복문
	{
		temp += buf[i];

		if (i == size - 1)     //남은 문자들을 모두 temp에 복사 후 pass1.operand에 저장한다.
		{
			pass1.operand = temp;
			break;
		}
		else i++;
	}
	return;
}

int SYM_DIV(int locc, CODE pass1, SYMBOL *pass1S)  //SYMTAB을 만들기 위한 함수
{
	pass1S->loc = 0;  //loc값을 0으로 초기화
	pass1S->symbol = "\0";
	if (pass1.label[0] != ' ') // pass1 구조체 변수의 LABEL이 공백이 아니고
	{
		if (pass1.label[0] != '.') // 주석도 아니면
		{
			if (SymbolNUM == 0)    //SYMTAB에 아무 값도 없을 경우
			{                      //LABEL과 위치계수기 값을 저장한다.
				pass1S[SymbolNUM].symbol = pass1.label; 
				pass1S[SymbolNUM].loc = locc;
			}
			else  //SYMTAB에 값이 들어가 있는 경우
			{
				for (int i = 0; i<SymbolNUM; i++)         //현재 SYMTAB에 존재하는 SYMBOL 수 만큼 반복
					if (pass1.label == pass1S[i].symbol)  //기존값과 비교해서 존재하면 리턴, 
						return 0;
				pass1S[SymbolNUM].symbol = pass1.label;   //존재하지 않으면 LABEL과 위치계수기 값 저장
				pass1S[SymbolNUM].loc = locc;
			}
			SymbolNUM++;                                  //SYMBOL 수 증가
			return 1;
		}
	}
}


//Location counter값을 계산할 때 명령어 뿐만 아니라 기억장소로 쓰이는 부분(assembler directive)까지 계산하여야 한다.
int LOC_Calculation(CODE pass1, int &loc)  //위치 계수기 값을 계산하는 함수
{
	int i;
	if (pass1.opcode == "BYTE")        //OPCODE가 BYTE인 경우 : 문자 혹은 16진수 상수를 생성한다.
	{
		if (pass1.operand[0] == 'X')          //OPERAND의 첫 문자가 X일 경우 -> 16진수 상수 생성
			loc += 1;
		else if (pass1.operand[0] == 'C')     //OPERAND의 첫 문자가 C일 경우 -> 문자 생성
			loc += (pass1.operand.size() - 3);
	}
	else if (pass1.opcode == "WORD")   //OPCODE가 WORD인 경우 : 한 워드 정수 상수를 생성한다.
		loc += 3;
	else if (pass1.opcode == "RESW")   //OPCODE가 RESW인 경우 : 지정해준 수만큼의 워드들을 예약한다.
		loc += (atoi(pass1.operand.c_str()) * 3);
	else if (pass1.opcode == "RESB")   //OPCODE가 RESB인 경우 : 지정해준 수만큼의 바이트들을 예약한다.
		loc += (atoi(pass1.operand.c_str()));
	else  //어셈블러 지시자가 아닌경우
	{
		for (i = 0; i < 25; i++) {            // OPTAB의 갯수만큼 반복 		
			string c = OPTAB[i].opcode;
			if (pass1.opcode.compare(c) == 0) //OPCODE가 OPTAB에 존재하는 경우
			{
				loc += 3;
				break;
			}
		}
		if (i == 25)
			if (pass1.label[0] != '.') {
				cout << "탐색 실패 "<< pass1.opcode << endl;
				return 0;  //탐색 실패
			}
	}
	return 1;  //탐색 성공
}

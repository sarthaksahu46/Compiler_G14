/*  

	SymbolTableDef.h 
	
	Batch No: 14

	Members: 

	Ishan Sharma 2016B2A70773P
	Sarthak Sahu 2015B5A70749P
	Anirudh Garg 2017A7PS0142P
	Sanjeev Singla 2017A7PS0152P

*/

/* END COLOR DEFINE */

#ifndef ST_DEFH
#define ST_DEFH

struct index
{
	int isused;
	char id[30];
	int ifnumvalue;
	int isDynamic;
	struct symbolnode *index;
};

typedef struct index Index;

struct symbolnode
{
	char name[30];

	int usage;

	// 1: Variable
	// 2. Function Def
	// 3. Input Parameters
	// 4. Output Parameters
	// 5. Function Declaration
	// 6. Function Def and Dec

	char type[30];
	int isArray;
	Index *startindex;
	Index *endindex;
	int offset;
	int width;
	int lineno;
	struct symbolnode *next;
	struct symboltable *scope; // Gets which scope this node is in
	int isAss;
	int mrsreq; // used to have the value of latest line no if func decl is required. 
	int declno; // used to store declaration line no of function 
	int deflno; // used to store the defination line no of function
	int udv; // flag for UDV
	char temporary[5]; // Temporary Variable used for SymbolEntry in CodeGen
};

typedef struct symbolnode SymbolEntry;

struct symboltable
{
	char name[30];

	struct symboltable *parent;
	struct symboltable *child;
	struct symboltable *left;
	struct symboltable *right;
	struct symbolnode *nodehead;
	int startlno;
	int endlno;

	//int offsetper;
};

typedef struct symboltable SymbolTable;

#endif
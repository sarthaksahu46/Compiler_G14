/*  

	SymbolTable.c 
	
	Batch No: 14

	Members: 

	Ishan Sharma 2016B2A70773P
	Sarthak Sahu 2015B5A70749P
	Anirudh Garg 2017A7PS0142P
	Sanjeev Singla 2017A7PS0152P

	// Symbol Table theek kar

*/

#include "ast.h"
#include "SymbolTable.h"

SymbolTable *globaltable;
int offsetinp = 0;
int offsetmodule = 0;
int offset = 0;
SymbolEntry *AddEntryWala;
SymbolTable *driver;

/********************************************************************************************************************************/
/********************************************************************************************************************************/

/* Auxilary Functions */

SymbolTable *ScopeEntry(SymbolTable *table, char *scopename, int line)
{
	SymbolTable *newTable = (SymbolTable *)malloc(sizeof(SymbolTable));
	strcpy(newTable->name, scopename);
	newTable->parent = NULL;
	newTable->child = NULL;
	newTable->right = NULL;
	newTable->left = NULL;
	newTable->nodehead = NULL;
	newTable->startlno = line;
	//newTable->endlno = 0;

	// Insert scope in tree

	if(table != NULL)
	{
		if(table->child == NULL)
			table->child = newTable;

		else
		{
			SymbolTable *sib = table->child;

			while(sib->right != NULL)
				sib = sib->right;

			sib->right = newTable;
			newTable->left = sib;
		}

		newTable->parent = table;
	}

	if((newTable->parent != NULL) && strcmp(newTable->parent->name,"global")==0)
	{
		offsetmodule = 0;
	}
	else if(strcmp(newTable->name,"input_plist")==0)
	{	
		offsetinp = 0;
	}

	return newTable;
}

SymbolEntry *FindEntryEverywhere(char *id, SymbolTable *scope, int line, int isFunc, int *errors)
{
	int found = 0; // not found

	SymbolTable *temptable = scope;
	SymbolEntry *templist;

	while(temptable != NULL)
	{

		if(strcmp(temptable->name,"global") == 0 && isFunc == 0)
		{
			break;
		}

		templist = temptable->nodehead;

		while(templist != NULL)
		{
			if(strcmp(templist->name, id) == 0)
			{
				return templist;
			}
			templist = templist->next;
		} 

		// Agar function mei nahi mila tabh check karega inputlist mei 

		if(isFunc == 0 && (strcmp(temptable->name,"driver")!=0) && (strcmp(temptable->parent->name,"global")==0)) // Here are all functions, this is to search in inputplist
		{

			SymbolTable *tempIPL = temptable->child; // inputplist
			SymbolEntry *temp = tempIPL->nodehead;

			while(temp != NULL)
			{
				if(strcmp(temp->name, id) == 0)
				{
					return temp;
				}
				temp = temp->next;
			}
		}
		temptable = temptable->parent;
	}
	return NULL;
}

int FindEntryInScope(char *id, SymbolTable *scope)
{
	SymbolEntry *entrylist = scope->nodehead;
	while(entrylist != NULL)
	{
		if(strcmp(entrylist->name,id) == 0)
		{
			return entrylist->usage;
		}
		entrylist = entrylist->next;
	}
	return 0;
}

void AddEntry(char *id, int usage, char *type, int isArray, Index *startindex, Index *endindex, int line, SymbolTable *scope, int *errors)
{
	// Search the current identifier in the scopes list

	SymbolEntry *entrylist = scope->nodehead;

	int foundornot = 0; // Not found anywhere

	int checkEntry = 0;

	checkEntry = FindEntryInScope(id, scope);

	if(checkEntry == 1 || checkEntry == 2 || checkEntry == 6)
	{
		if(strcmp(type,"N.A")==0)
		{
			printf("\t%sLine No: %d%s (Error) %sThe function '%s' cannot be overloaded in the same scope (%s).\n", BOLDWHITE, line, BOLDRED, RESET, id, scope->name);
			foundornot = 0; // FOUND already, therefore don't insert in table
		}
		else
		{
			printf("\t%sLine No: %d%s (Error) %sThe identifier '%s' was declared multiple times in the same scope (%s).\n", BOLDWHITE, line, BOLDRED, RESET, id,scope->name);
			foundornot = 1; // FOUND already, therefore don't insert in table
			*errors = 1;
		}
	}
	else if(checkEntry == 3)
	{
		printf("\t%sLine No: %d%s (Error) %sThe identifier '%s' cannot be declared as it is being used as Input Parameters in the same scope (%s).\n", BOLDWHITE, line, BOLDRED, RESET, id,scope->name);
		foundornot = 1; // FOUND already, therefore don't insert in table
		*errors = 1;
	}
	else if(checkEntry == 4)
	{
		printf("\t%sLine No: %d%s (Error) %sThe identifier '%s' cannot be declared as it is being used as Output Parameters in the same scope (%s).\n", BOLDWHITE, line, BOLDRED, RESET, id,scope->name);
		foundornot = 1; // FOUND already, therefore don't insert in table
		*errors = 1;
	}
	else if(checkEntry == 5)
	{
		if(usage == 2)
		{
			SymbolEntry *entrylist1 = scope->nodehead;
			while(entrylist1 != NULL)
			{
				if(strcmp(entrylist1->name,id) == 0)
				{
					entrylist1->usage = 6;
					foundornot = 1;

					entrylist1->deflno = line;
				}
				entrylist1 = entrylist1->next;
			}
			//foundornot = 0;
		}
		else
		{
			printf("\t%sLine No: %d%s (Error) %sThe identifier '%s' cannot declared multiple times in the same scope (%s).\n", BOLDWHITE, line, BOLDRED, RESET, id, scope->name);
			foundornot = 1; // FOUND already, therefore don't insert in table
			*errors = 1;
		}
	}

	// Flag trigger nahi hua isslie ghusao

	if(foundornot == 0) // Entry not found in table
	{
		// printf("\nHello Sar Inside Entry Not Found\n");

		SymbolEntry *newEntry = (SymbolEntry *)malloc(sizeof(SymbolEntry));
		
		strcpy(newEntry->name, id);
		newEntry->usage = usage;
		newEntry->isArray = isArray;
		strcpy(newEntry->type,type);
		newEntry->startindex = startindex;
		newEntry->endindex = endindex;
		newEntry->offset = 0;
		newEntry->width = 0;
		newEntry->lineno = line;
		newEntry->scope = scope;
		newEntry->mrsreq = -1;
		newEntry->declno = -1;
		newEntry->deflno = -1;
		newEntry->udv = 0;

		if(usage == 2)
		{
			newEntry->deflno = line;
		}

		if(usage == 5)
		{
			newEntry->declno = line;
		}

		// Calculate offset and width now: 

		if(scope->parent != NULL) // Some Nesting not gobal
		{
			if(usage == 3 || usage == 4)
			{
				newEntry->offset = offsetinp;

				int ei = endindex->ifnumvalue;
				int si = startindex->ifnumvalue;

				if(isArray)
				{
					if(startindex->isDynamic == 0 && endindex->isDynamic == 0)
					{
						int dt = 0;

						if(strcmp(type,"INTEGER") == 0)
							dt = 2;

						else if(strcmp(type,"REAL") == 0)
							dt = 4;

						else if(strcmp(type,"BOOLEAN") == 0)
							dt = 1;

						if(usage == 3)
						{
							//printf("\nID: %s |\n", newEntry->name);
							newEntry->width = 5;
							offsetinp += newEntry->width;
						}
						else
						{
							newEntry->width = 1 + (ei-si+1) * dt;
							offsetinp += newEntry->width;
						}
					}
					else
					{
						
						newEntry->width = 5;
						offsetinp += newEntry->width;
					}

					goto skipwidth1;
				}

				if(strcmp(type,"INTEGER") == 0)
				{
					newEntry->width = 2;
					offsetinp += newEntry->width;
				}
				else if(strcmp(type,"REAL") == 0)
				{
					newEntry->width = 4;
					offsetinp += newEntry->width;
				}
				else if(strcmp(type,"BOOLEAN") == 0)
				{
					newEntry->width = 1;
					offsetinp += newEntry->width;
				}
				skipwidth1: ;
			}
			else
			{
				newEntry->offset = offsetmodule;

				int ei = endindex->ifnumvalue;
				int si = startindex->ifnumvalue;

				if(isArray)
				{
					if(startindex->isDynamic == 0 && endindex->isDynamic == 0)
					{
						int dt = 0;

						if(strcmp(type,"INTEGER") == 0)
							dt = 2;

						else if(strcmp(type,"REAL") == 0)
							dt = 4;

						else if(strcmp(type,"BOOLEAN") == 0)
							dt = 1;

						if(usage == 3)
						{
							//printf("\nID: %s |\n", newEntry->name);
							newEntry->width = 5;
							offsetmodule += newEntry->width;
						}
						else
						{
							newEntry->width = 1 + (ei-si+1) * dt;
							offsetmodule += newEntry->width;
						}
					}
					else
					{
						
						newEntry->width = 1;
						offsetmodule += newEntry->width;
					}

					goto skipwidth2;
				}

				if(strcmp(type,"INTEGER") == 0)
				{
					newEntry->width = 2;
					offsetmodule += newEntry->width;
				}
				else if(strcmp(type,"REAL") == 0)
				{
					newEntry->width = 4;
					offsetmodule += newEntry->width;
				}
				else if(strcmp(type,"BOOLEAN") == 0)
				{
					newEntry->width = 1;
					offsetmodule += newEntry->width;
				}
				skipwidth2: ;
			}	
		}

		SymbolEntry *nh = scope->nodehead;

		if(nh == NULL)
		{
			scope->nodehead = newEntry;
			newEntry->next = NULL;
		}
		else
		{
			SymbolEntry *temp = nh;

			while(temp->next != NULL)
				temp = temp->next;

			temp->next = newEntry;
			newEntry->next = NULL;
		}

		AddEntryWala = newEntry;
	}
}

void ReassignMRS(ParseTree *head, SymbolTable *globaltable, int *errors)
{
	if(head == NULL)
		return;

	SymbolEntry *found;

	if(strcmp(terms[(head->value)],"ID") == 0)
	{
		if(strcmp(terms[(head->parent->value)],"moduleReuseStmt") == 0)
		{
			//printf("\nGoing in: MRSID: %s | Global Table: %s\n", head->n->t->value, globaltable->name);
			found = FindEntryEverywhere(head->n->t->value, globaltable, head->n->t->lineno, 1, errors);

			head->entry = found;

			if(found != NULL)
			{
				//printf("\nCame here for MRSID: %s\n", head->n->t->value);
				if(found->usage == 6 && (found->deflno > head->n->t->lineno))
				{
					found->mrsreq = head->n->t->lineno;
				}
			}
		}
	}

	ReassignMRS(head->child, globaltable, errors);
	ReassignMRS(head->right, globaltable, errors);
}

/* Auxilary Functions END */

/********************************************************************************************************************************/
/********************************************************************************************************************************/

/* MAIN Functions */

void recActiveRecords(SymbolTable *func, int *tw, char *funcname)
{
	if(func == NULL)
		return;

	SymbolEntry *templist = func->nodehead;

	while(templist != NULL)
	{
		if(templist->usage !=3 && templist->usage !=4)
			*tw = *tw + templist->width;

		templist = templist->next;
	}

	recActiveRecords(func->child, tw, funcname);
	if(func->child != NULL)
		recActiveRecords(func->child->right, tw, funcname);
}

void printActiveRecords(SymbolTable *Table)
{
	SymbolTable *temptable = Table->child;

	while(temptable != NULL)
	{
		int totalwidth = 0;

		char funcname[30];

		strcpy(funcname,temptable->name);

		recActiveRecords(temptable, &totalwidth, funcname);

		printf("%-20s \t %d\n", temptable->name, totalwidth);

		temptable = temptable->right;
	}
}

void printSymbolTableArray(SymbolTable *table)
{
	if(table == NULL)
		return;

	SymbolTable *childtable;
	SymbolEntry *EntryList;
	int nesting = 0;

	EntryList = table->nodehead;

	SymbolTable *nest = table;
	SymbolTable *sn = table;

	while(nest->parent != NULL)
	{
		nesting++;
		nest = nest->parent;
	}

	while(EntryList != NULL)
	{
		if(EntryList->isArray)
		{
			char scopename[30];

			char idname[30];
			strcpy(idname, EntryList->name);

			int usage = EntryList->usage;
			char usagename[30];

			while(strcmp(sn->parent->name,"global") != 0)
				sn = sn->parent;

			strcpy(scopename,sn->name);

			switch(usage)
			{
				case 1: strcpy(usagename,"Variable");
						break;

				case 2: strcpy(usagename,"FuncDefin");
						break;

				case 3: strcpy(usagename,"InputList");
						break; 

				case 4: strcpy(usagename,"OutputList");
						break;

				case 5: strcpy(usagename,"FuncDeclar");
						break;

				case 6: strcpy(usagename,"FunDefDecl");
						break;

				default:strcpy(usagename,"** NONE **");
						break;
			}

			char type[30];
			strcpy(type, EntryList->type);

			char arraytype[30];
			char isA[20];
			char isS[20];

			if(EntryList->isArray == 1)
			{
				strcpy(isA,"Yes");

				strcpy(arraytype,"[");

				char index1[10];
				char index2[10];

				if(EntryList->startindex->isDynamic == 0 && EntryList->endindex->isDynamic == 0)
					strcpy(isS,"Static");
				else
					strcpy(isS,"Dynamic");

				if(EntryList->startindex->isDynamic == 0)
				{
					sprintf(index1,"%d,",EntryList->startindex->ifnumvalue);
				}
				else
				{
					strcpy(index1,EntryList->startindex->id);
					strcat(index1,",");
				}
				if(EntryList->endindex->isDynamic == 0)
				{
					sprintf(index2,"%d]",EntryList->endindex->ifnumvalue);
				}
				else
				{
					strcpy(index2,EntryList->endindex->id);
					strcat(index2,"]");
				}

				char cominx[30];

				strcpy(cominx,index1);
				strcat(cominx,index2);
				strcat(arraytype,cominx);
			}
			else
			{
				strcpy(isA,"No");
				strcpy(arraytype,"----");
				strcpy(isS,"----");
			}

			int lineno = EntryList->lineno;

			char scopeparent[30];

			if(EntryList->scope->parent == NULL)
				strcpy(scopeparent, "NULL");
			else
				strcpy(scopeparent, EntryList->scope->parent->name);

			/*if(usage == 3)
			{
				table->endlno = table->parent->endlno;
			}*/

			if(usage == 4)
			{
				strcpy(scopename,"output_plist");
				nesting = 2;
				strcpy(scopeparent,EntryList->scope->name);
			}

			int width = EntryList->width;
			int offset = EntryList->offset;

			int slno = EntryList->lineno;
			//int elno = EntryList->scope->endlno;

			printf("%-20s %-10d %-20s %-10s %-20s %-10s\n", scopename, slno, idname, isS, arraytype, type);

		}
		EntryList = EntryList->next;
	}

	printSymbolTableArray(table->child);
	printSymbolTableArray(table->right);

}

void printSymbolTable(SymbolTable *table)
{
	if(table == NULL)
		return;

	if(table->parent == NULL)
		goto skipglobal;

	SymbolTable *childtable;
	SymbolEntry *EntryList;

	EntryList = table->nodehead;

	SymbolTable *sn = table;

	while(EntryList != NULL)
	{
		int nesting = 0;
		SymbolTable *nest = table;
		while(nest->parent != NULL)
		{
			nesting++;
			nest = nest->parent;
		}

		char scopename[30];

		while(strcmp(sn->parent->name,"global") != 0)
			sn = sn->parent;

		strcpy(scopename,sn->name);

		char idname[30];
		strcpy(idname, EntryList->name);

		int usage = EntryList->usage;

		char usagename[30];

		switch(usage)
		{
			case 1: strcpy(usagename,"Variable");
					break;

			case 2: strcpy(usagename,"FuncDefin");
					break;

			case 3: strcpy(usagename,"InputList");
					break; 

			case 4: strcpy(usagename,"OutputList");
					break;

			case 5: strcpy(usagename,"FuncDeclar");
					break;

			case 6: strcpy(usagename,"FunDefDecl");
					break;

			default:strcpy(usagename,"** NONE **");
					break;
		}

		if(EntryList->usage == 4 || EntryList->usage == 3)
		{
			nesting = 0;
		}

		char type[30];
		strcpy(type, EntryList->type);

		char arraytype[30];
		char isA[20];
		char isS[20];

		if(EntryList->isArray == 1)
		{
			strcpy(isA,"Yes");

			strcpy(arraytype,"[");

			char index1[10];
			char index2[10];

			if(EntryList->startindex->isDynamic == 0 && EntryList->endindex->isDynamic == 0)
				strcpy(isS,"Static");
			else
				strcpy(isS,"Dynamic");

			if(EntryList->startindex->isDynamic == 0)
			{
				sprintf(index1,"%d,",EntryList->startindex->ifnumvalue);
			}
			else
			{
				strcpy(index1,EntryList->startindex->id);
				strcat(index1,",");
			}
			if(EntryList->endindex->isDynamic == 0)
			{
				sprintf(index2,"%d]",EntryList->endindex->ifnumvalue);
			}
			else
			{
				strcpy(index2,EntryList->endindex->id);
				strcat(index2,"]");
			}

			char cominx[30];

			strcpy(cominx,index1);
			strcat(cominx,index2);
			strcat(arraytype,cominx);
		}
		else
		{
			strcpy(isA,"No");
			strcpy(arraytype,"----");
			strcpy(isS,"----");
		}

		//strcpy(scopename,EntryList->scope->name);

		/*if(usage == 3)
		{
			table->endlno = table->parent->endlno;
			nesting = 1;
		}*/

		int slno = EntryList->lineno;
		//int elno = EntryList->scope->endlno;

		int width = EntryList->width;
		int offset = EntryList->offset;

		printf("%-20s %-20s %-3d \t %d \t %-5s    %-10s    %-10s    %-10s %d \t %d\n", idname, scopename, slno, width,isA,isS,arraytype,type,offset,nesting);

		//printf("%-6s \t\t %-10s \t %d \t %d \t %-6s %-6s \t\t %-6s \t %-6s \t %d \t %d\n", idname, scopename, lineno, width,isA,isS,arraytype,type,offset,nesting);

		EntryList = EntryList->next;
	}

	skipglobal: ;

	printSymbolTable(table->child);
	printSymbolTable(table->right);

}

void TypeNULLinit(ParseTree *head)
{
	if(head == NULL)
		return;

	strcpy(head->type,"NULL");

	TypeNULLinit(head->child);
	TypeNULLinit(head->right);
}

SymbolTable *CallingSymbolTable(ParseTree *head, int *errors, int *udvflag)
{
	TypeNULLinit(head);

	SymbolTable *table;

	table = ScopeEntry(NULL, "global", 0);

	globaltable = table;

	printf("\n\n");

	//printf("\n1. Error here\n");

	ConstructSymbolTable(head, table, errors, udvflag);

	ReassignMRS(head, globaltable, errors);

	return table;
}

void ConstructSymbolTable(ParseTree *headroot, SymbolTable *scope, int *errors, int *udvflag)
{
	SymbolTable *newScope;
	SymbolEntry *newEntry;

	ParseTree *head;

	headroot->scope = scope;

	// Head is any ID
	if(headroot->child != NULL)
	{
		head = headroot->child;

		while(head != NULL)
		{
			head->scope = scope;
			//printf("\nHead: %s | Scope: %s | ScopeP: %s\n",terms[head->value],head->scope->name, head->scope->parent->name);

			if(strcmp(terms[(head->value)],"ID") == 0)
			{
				if((strcmp(terms[(head->parent->value)],"idList") == 0) && (strcmp(terms[(head->parent->parent->value)],"declareStmt") == 0))
				{

					ParseTree *datatype = head->parent->right->child;

					if(strcmp(terms[(datatype->value)],"ARRAY") == 0)
					{
						//printf("\n\nARRAY ID: %s | Scope: %s", head->n->t->value, scope->name);
						//printf("\n\nInside array\n\n");

						ParseTree *index1 = head->parent->right->child->right->child->child;
						ParseTree *index2 = head->parent->right->child->right->child->right->child;
						ParseTree *datatypearray = head->parent->right->child->right->right;

						Index *i1 = (Index *)malloc(sizeof(Index));
						Index *i2 = (Index *)malloc(sizeof(Index));

						i1->isused = 1;
						i2->isused = 1;

						if(strcmp(terms[index1->value],"ID") == 0)
						{
							strcpy(i1->id,index1->n->t->value);
							i1->ifnumvalue = -1;
							i1->isDynamic = 1;
						}
						else // INDEX1 is num
						{
							int value = atoi(index1->n->t->value);
							strcpy(i1->id,"NUM");
							i1->ifnumvalue = value;
							i1->isDynamic = 0;
						}

						if(strcmp(terms[index2->value],"ID") == 0)
						{
							strcpy(i2->id,index2->n->t->value);
							i2->ifnumvalue = -1;
							i2->isDynamic = 1;
						}
						else // INDEX2 is num
						{
							int value = atoi(index2->n->t->value);
							strcpy(i2->id,"NUM");
							i2->ifnumvalue = value;
							i2->isDynamic = 0;
						}

						//             ID name   Usage: Variable  Datatype  isArray
						AddEntry(head->n->t->value, 1, terms[datatypearray->value], 1, i1, i2, head->n->t->lineno, scope, errors);
						head->entry = AddEntryWala;
						strcpy(head->type, AddEntryWala->type);
					}
					else // Non array INTEGER | REAL | BOOLEAN 
					{	
						//printf("\nHello Sar Inside Datatype \n");
						//printf("\n\n%s ID: %s | Scope: %s",terms[datatype->value], head->n->t->value, scope->name);

						Index *i1 = (Index *)malloc(sizeof(Index));
						Index *i2 = (Index *)malloc(sizeof(Index));

						i1->isused = 0;
						i2->isused = 0;

						i1->ifnumvalue = -1;
						i2->ifnumvalue = -1;

						//printf("\n\nID: %s | Datatype: %s | Scopename: %s\n\n",head->n->t->value, terms[datatype->value], scope->name);
						
						//              ID name  Usage: Variable  Datatype  Not array 
						AddEntry(head->n->t->value, 1, terms[datatype->value], 0, i1, i2, head->n->t->lineno, scope, errors);
						head->entry = AddEntryWala;
						strcpy(head->type, AddEntryWala->type);
					}
				}
				else if(strcmp(terms[(head->parent->value)],"input_plist") == 0)
				{

					ParseTree *datatype = head->right->child;

					//printf("\nID: %s | DT: %s\n",head->n->t->value,terms[datatype->value]);

					if(strcmp(terms[(datatype->value)],"ARRAY") == 0)
					{

						ParseTree *index1 = head->right->child->right->child->child;
						ParseTree *index2 = head->right->child->right->child->right->child;
						ParseTree *datatypearray = head->right->child->right->right;

						Index *i1 = (Index *)malloc(sizeof(Index));
						Index *i2 = (Index *)malloc(sizeof(Index));

						i1->isused = 1;
						i2->isused = 1;

						if(strcmp(terms[index1->value],"ID") == 0)
						{
							strcpy(i1->id,index1->n->t->value);
							i1->ifnumvalue = -1;
							i1->isDynamic = 1;
						}
						else // INDEX1 is num
						{
							int value = atoi(index1->n->t->value);
							strcpy(i1->id,"NUM");
							i1->ifnumvalue = value;
							i1->isDynamic = 0;
						}

						if(strcmp(terms[index2->value],"ID") == 0)
						{
							strcpy(i2->id,index2->n->t->value);
							i2->ifnumvalue = -1;
							i2->isDynamic = 1;
						}
						else // INDEX2 is num
						{
							int value = atoi(index2->n->t->value);
							strcpy(i2->id,"NUM");
							i2->ifnumvalue = value;
							i2->isDynamic = 0;
						}

						//             ID name   Usage: input_plist  Datatype  isArray
						AddEntry(head->n->t->value, 3, terms[datatypearray->value], 1, i1, i2, head->n->t->lineno, scope, errors);
						head->entry = AddEntryWala;
						strcpy(head->type, AddEntryWala->type);

						//printf("\nyahan end hoke bakchodi ho rahi hai\n");
					}
					else // Non array INTEGER | REAL | BOOLEAN 
					{	
						//printf("\n\n%s IPL: %s | Scope: %s | ScopeParent: %s", terms[datatype->value], head->n->t->value, scope->name, scope->parent->name);

						//printf("\nHello Sar Inside Datatype \n");
						Index *i1 = (Index *)malloc(sizeof(Index));
						Index *i2 = (Index *)malloc(sizeof(Index));

						i1->isused = 0;
						i2->isused = 0;

						i1->ifnumvalue = -1;
						i2->ifnumvalue = -1;

						//printf("\n\nID: %s | Datatype: %s | Scopename: %s\n\n",head->n->t->value, terms[datatype->value], scope->name);
						
						//              ID name  Usage: input_plist  Datatype  Not array 
						AddEntry(head->n->t->value, 3, terms[datatype->value], 0, i1, i2, head->n->t->lineno, scope, errors);
						head->entry = AddEntryWala;
						strcpy(head->type, AddEntryWala->type);
					}
				}
				else if(strcmp(terms[(head->parent->value)],"output_plist") == 0)
				{

					ParseTree *datatype = head->right;
			
					// Output_p_list array ho hi nahi sakta issliye array wala bye bye,

					Index *i1 = (Index *)malloc(sizeof(Index));
					Index *i2 = (Index *)malloc(sizeof(Index));

					i1->isused = 0;
					i2->isused = 0;

					i1->ifnumvalue = -1;
					i2->ifnumvalue = -1;

					//printf("\n\n%s OPL: %s | Scope: %s | ScopeParent: %s", terms[datatype->value], head->n->t->value, scope->name, scope->parent->name);

					//printf("\n\nID: %s | Datatype: %s | Scopename: %s\n\n",head->n->t->value, terms[datatype->value], scope->name);
					
					//              ID name  Usage: output_plist  Datatype  Not array 
					AddEntry(head->n->t->value, 4, terms[datatype->value], 0, i1, i2, head->n->t->lineno, scope, errors);
					head->entry = AddEntryWala;
					strcpy(head->type, AddEntryWala->type);
				}
				
				else if(strcmp(terms[(head->parent->value)],"module") == 0)
				{
					Index *i1 = (Index *)malloc(sizeof(Index));
					Index *i2 = (Index *)malloc(sizeof(Index));

					i1->isused = 0;
					i2->isused = 0;

					i1->ifnumvalue = -1;
					i2->ifnumvalue = -1;

					AddEntry(head->n->t->value, 2, "N.A", 0, i1, i2, head->n->t->lineno, globaltable, errors);
					head->entry = AddEntryWala;
					strcpy(head->type, AddEntryWala->type);
				}
				else if(strcmp(terms[(head->parent->value)],"moduleDeclarations") == 0)
				{
					Index *i1 = (Index *)malloc(sizeof(Index));
					Index *i2 = (Index *)malloc(sizeof(Index));

					i1->isused = 0;
					i2->isused = 0;

					i1->ifnumvalue = -1;
					i2->ifnumvalue = -1;

					AddEntry(head->n->t->value, 5, "N.A", 0, i1, i2, head->n->t->lineno, scope, errors);
					head->entry = AddEntryWala;
					strcpy(head->type, AddEntryWala->type);
				}
				else
				{
					SymbolEntry *found;

					if(strcmp(terms[(head->parent->value)],"moduleReuseStmt") == 0)
					{
						/*found = FindEntryEverywhere(head->n->t->value, scope, head->n->t->lineno, 1, errors);
						head->entry = found;

						if(found != NULL)
						{
							//printf("\nMRS at lineno: %d | Func: %s | FuncLno: %d | FuncUsage: %d\n",head->n->t->lineno, head->entry->name, head->entry->lineno, head->entry->usage);
							if(found->usage == 5)
							{
								//printf("\nMRS at lineno: %d | Func: %s | FuncLno: %d | FuncUsage: %d\n",head->n->t->lineno, head->entry->name, head->entry->lineno, head->entry->usage);
								found->mrsreq = head->n->t->lineno;
							}
						}*/
					}
					else
					{

						found = FindEntryEverywhere(head->n->t->value, scope, head->n->t->lineno, 0, errors);
						
						if(found == NULL)
						{
							printf("%s\tLine No: %d %s(Error)%s The identifier '%s' should be declared before its use.\n", BOLDWHITE, head->n->t->lineno,BOLDRED, RESET, head->n->t->value);
							*errors = 1;

							head->entry = (SymbolEntry *)malloc(sizeof(SymbolEntry));
							head->entry->udv = 1;
						}
						else
						{
							head->entry = found;
							strcpy(head->type,found->type);
						}
					} 
				}
			}
			else
			{
				if(strcmp(terms[(head->value)],"conditionalStmt") == 0)
				{
					newScope = ScopeEntry(scope, head->child->n->t->value,head->child->n->t->lineno);
					ConstructSymbolTable(head, newScope, errors, udvflag);
				}
				else if(strcmp(terms[(head->value)],"module") == 0)
				{
					newScope = ScopeEntry(scope, head->child->n->t->value,head->child->n->t->lineno);
					ConstructSymbolTable(head, newScope, errors, udvflag);
				}
				else if(strcmp(terms[(head->value)],"driverModule") == 0)
				{
					newScope = ScopeEntry(scope, "driver",head->child->n->t->lineno);
					driver = newScope;
					ConstructSymbolTable(head, newScope, errors, udvflag);
				}
				else if(strcmp(terms[(head->value)],"iterativeStmt") == 0)
				{
					newScope = ScopeEntry(scope, head->child->n->t->value,head->child->n->t->lineno);
					ConstructSymbolTable(head, newScope, errors, udvflag);
				}
				else if(strcmp(terms[(head->value)],"input_plist") == 0)
				{
					newScope = ScopeEntry(scope, "input_plist",head->child->n->t->lineno);
					ConstructSymbolTable(head, newScope, errors, udvflag);
				}
				else if(strcmp(terms[(head->value)],"END") == 0)
				{
					//scope->endlno = head->n->t->lineno;
				}
				else
				{
					ConstructSymbolTable(head, scope, errors, udvflag);
				}
			}
			head = head->right;
		}
	}
}

/* MAIN Functions END */ 

/********************************************************************************************************************************/
/********************************************************************************************************************************/
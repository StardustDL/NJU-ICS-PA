#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <ctype.h>

enum
{
	TK_NOTYPE = 256,
	TK_EQ = 257, TK_NUMD = 258, TK_NUMH = 259, TK_NUMB = 260, TK_NUMO = 261, TK_NEQ = 262, TK_AND = 263, TK_REG = 264,
	TK_NEG = 512, TK_POINT = 513,
	/* TODO: Add more token types */

};

const int PRI_NEG = 50, PRI_POINT = 60;

static struct rule
{
	char *regex;
	int token_type;
	int opPri;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", TK_NOTYPE, 0},	// spaces
	{"\\+", '+', 10},	// plus
	{"-", '-', 10},		// minus
		// - for neg: pri=50
	{"\\*", '*', 20},	// mul or point
	{"/", '/', 20},		// divide
	{"==", TK_EQ, 5},	// equal
	{"!=", TK_NEQ, 5},	// not equal
	{"&&", TK_AND, 8},	// and
	{"\\$([A-Z]|[a-z])+", TK_REG, 0},	// registers
	{"\\(", '(', 0}, 
	{"\\)", ')', 0},
		//{"(-){0,1}[0-9]+", TK_NUMD, 0},  // decimal integer
	{"0x{0,1}([0-9]|[A-F]|[a-f])+", TK_NUMH, 0}, 
	{"0b{0,1}([0-1])+", TK_NUMB, 0}, 
	{"0o([0-1])+", TK_NUMO, 0}, 
	{"(-){0,1}[0-9]+", TK_NUMD, 0},	// decimal integer
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if (ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			panic("regex compilation failed: %s\n%s", error_msg,
			      rules[i].regex);
		}
	}
}

static uint32_t toInteger(char *s, uint32_t base)
{
	bool neg = s[0] == '-';
	uint32_t ans = 0;
	for (int i = neg; s[i]; i++) {
		uint32_t t = 0;
		if (isdigit(s[i]))
			t = s[i] - '0';
		else if (islower(s[i]))
			t = s[i] - 'a' + 10;
		else if (isupper(s[i]))
			t = s[i] - 'A' + 10;
		else
			panic("Unexceped char when parse string to int");
		ans = ans * base + t;
	}
	if (neg)
		ans = -ans;
	return ans;
}

typedef struct token
{
	int type;
	bool isOp;
	bool isValue;
	char str[32];
	union
	{
		uint32_t data;
		int priority;
	};
} Token;

Token tokens[100000];
int nr_token;

static bool make_token(char *e)
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0') {
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++) {
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0
			    && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

/*        Info("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);*/
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch (rules[i].token_type) {
				case TK_NOTYPE:
					break;
				case TK_EQ:{
						Token *ctk = &tokens[nr_token];
						ctk->type = TK_EQ;
						ctk->isOp = true;
						ctk->isValue = false;
						ctk->priority = rules[i].opPri;
						nr_token++;
					}
					break;
				case TK_NEQ:{
						Token *ctk = &tokens[nr_token];
						ctk->type = TK_NEQ;
						ctk->isOp = true;
						ctk->isValue = false;
						ctk->priority = rules[i].opPri;
						nr_token++;
					}
					break;
				case TK_AND:{
						Token *ctk = &tokens[nr_token];
						ctk->type = TK_AND;
						ctk->isOp = true;
						ctk->isValue = false;
						ctk->priority = rules[i].opPri;
						nr_token++;
					}
					break;
				case '+':{
						Token *ctk = &tokens[nr_token];
						ctk->type = '+';
						ctk->isOp = true;
						ctk->isValue = false;
						ctk->priority = rules[i].opPri;
						nr_token++;
					}
					break;
				case '-':{
						Token *ctk =
							&tokens[nr_token];
						if (nr_token - 1 < 0|| !tokens[nr_token - 1].isValue) {
							ctk->type = TK_NEG;
							ctk->priority =
								PRI_NEG;
						}
						else {
							ctk->type = '-';
							ctk->priority =
								rules[i].
								opPri;
						}
						ctk->isOp = true;
						ctk->isValue = false;
						nr_token++;
					}
					break;
				case '*':{
						Token *ctk =
							&tokens[nr_token];
						if (nr_token - 1 < 0|| !tokens[nr_token - 1].isValue) {
							ctk->type = TK_POINT;
							ctk->priority =
								PRI_POINT;
						}
						else {
							ctk->type = '*';
							ctk->priority =
								rules[i].
								opPri;
						}
						ctk->isOp = true;
						ctk->isValue = false;
						nr_token++;
					}
					break;
				case '/':{
						Token *ctk = &tokens[nr_token];
						ctk->type = '/';
						ctk->isOp = true;
						ctk->isValue = false;
						ctk->priority =
							rules[i].opPri;
						nr_token++;
					}
					break;
				case '(':{
						Token *ctk = &tokens[nr_token];
						ctk->type = '(';
						ctk->isOp = false;
						ctk->isValue = false;
						ctk->priority =
							rules[i].opPri;
						nr_token++;
					}
					break;
				case ')':{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = ')';
						ctk->isOp = false;
						ctk->isValue = false;
						ctk->priority =
							rules[i].opPri;
						nr_token++;
					}
					break;
					/*case '-':{
					   Token* ctk = &tokens[nr_token];
					   ctk->type = '-';
					   ctk->isOp = true;
					   ctk->priority = rules[i].opPri;
					   nr_token++;
					   }
					   break;
					   case '-':{
					   Token* ctk = &tokens[nr_token];
					   ctk->type = '-';
					   ctk->isOp = true;
					   ctk->priority = rules[i].opPri;
					   nr_token++;
					   }
					   break;
					   case '-':{
					   Token* ctk = &tokens[nr_token];
					   ctk->type = '-';
					   ctk->isOp = true;
					   ctk->priority = rules[i].opPri;
					   nr_token++;
					   }
					   break; */
				case TK_REG:{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = TK_REG;
						ctk->isOp = false;
						ctk->isValue = true;
						memcpy(ctk->str, substr_start,
						       sizeof(char) *
						       substr_len);
						ctk->str[substr_len] = '\0';
						nr_token++;
					}
					break;
				case TK_NUMD:{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = TK_NUMD;
						ctk->isOp = false;
						ctk->isValue = true;
						memcpy(ctk->str, substr_start,
						       sizeof(char) *
						       substr_len);
						ctk->str[substr_len] = '\0';
						ctk->data =
							toInteger(ctk->str,
								  10);
						nr_token++;
					}
					break;
				case TK_NUMH:{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = TK_NUMD;
						ctk->isOp = false;
						ctk->isValue = true;
						memcpy(ctk->str, substr_start,
						       sizeof(char) *
						       substr_len);
						ctk->str[substr_len] = '\0';
						ctk->data =
							toInteger(ctk->str +
								  2, 16);
						nr_token++;
					}
					break;
				case TK_NUMB:{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = TK_NUMB;
						ctk->isOp = false;
						ctk->isValue = true;
						memcpy(ctk->str, substr_start,
						       sizeof(char) *
						       substr_len);
						ctk->str[substr_len] = '\0';
						ctk->data =
							toInteger(ctk->str +
								  2, 2);
						nr_token++;
					}
					break;
				case TK_NUMO:{
						Token *ctk =
							&tokens[nr_token];
						ctk->type = TK_NUMO;
						ctk->isOp = false;
						ctk->isValue = true;
						memcpy(ctk->str, substr_start,
						       sizeof(char) *
						       substr_len);
						ctk->str[substr_len] = '\0';
						ctk->data =
							toInteger(ctk->str +
								  2, 8);
						nr_token++;
					}
					break;
				default:
					Error("Some unsolved token: %d",
					      rules[i].token_type);
					break;
				}

				break;
			}
		}

		if (i == NR_REGEX) {
			Error("no match at position %d\n%s\n%*.s^\n",
			      position, e, position, "");
			return false;
		}
	}

	return true;
}

static bool checkExtraP(int l, int r)
{
	if (tokens[l].type != '(')
		return false;
	if (tokens[r].type != ')')
		return false;
	int tl = 0;
	for (int i = l + 1; i < r; i++) {
		if (tokens[i].type == '(')
			tl++;
		else if (tokens[i].type == ')') {
			tl--;
			if (tl < 0)
				return false;
		}
	}
	return true;
}

static struct regMap
{
	char str[5];
	int offset;
} regmps[24] = {
	{"eax", 0}, 
	{"ah", 0}, 
	{"al", 0}, 
	{"ax", 0}, 
	{"ebx", 3}, 
	{"bh", 3}, 
	{"bl", 3}, 
	{"bx", 3}, 
	{"ecx", 1}, 
	{"ch", 1}, 
	{"cl", 1}, 
	{"cx", 1}, 
	{"edx", 2}, 
	{"dh", 2}, 
	{"dl", 2}, 
	{"dx", 2}, 
	{"ebp", 5}, 
	{"bp", 5}, 
	{"esi", 6}, 
	{"si", 6}, 
	{"edi", 7}, 
	{"di", 7}, 
	{"esp", 4}, 
	{"sp", 4},};

static uint32_t getReg(char *str, bool * success)
{
	*success = true;
	if (str == NULL) {
		Error("No register name");
		*success = false;
		return 0;
	}
	if (strcmp("eip", str) == 0) {
		return cpu.eip;
	}
	for (int i = 0; i < 24; i++) {
		if (strcmp(regmps[i].str, str) == 0) {
			switch (strlen(regmps[i].str)) {
			case 3:
				return cpu.gpr[regmps[i].offset]._32;
			case 2:
				switch (regmps[i].str[1]) {
				case 'x':
					return cpu.gpr[regmps[i].offset]._16;
				case 'h':
					return cpu.gpr[regmps[i].offset].
						_8[1];
				case 'l':
					return cpu.gpr[regmps[i].offset].
						_8[0];
				}
				break;
			}
		}
	}
	*success = false;
	Error("No this register");
	return 0;
}

uint32_t evalWithToken(int l, int r, bool * success)
{
	if (l > r) {
		panic("Expression evaluating failed");
	}
	if (l == r) {
		Token *ctk = tokens + l;
		switch (ctk->type) {
		case TK_NUMD:
		case TK_NUMH:
		case TK_NUMB:
		case TK_NUMO:
			*success = true;
			return ctk->data;
		case TK_REG:
			return getReg(ctk->str + 1, success);
		}
		panic("Expression evaluating failed");
	}
	//Check if the () is meaningless
	if (checkExtraP(l, r)) {
		return evalWithToken(l + 1, r - 1, success);
	}

	//Find the last op
	int tl = 0, pos = -1, mnPri = 1 << 30;
	for (int i = l; i <= r; i++) {
		if (tokens[i].type == '(') {
			tl++;
			continue;
		}
		else if (tokens[i].type == ')') {
			tl--;
			if (tl < 0) {
				Warning("Token: %d ,Too many ')'", i);
				*success = false;
				return 0;
			}
			continue;
		}
		if (tl || tokens[i].isOp == false)
			continue;
		if (tokens[i].priority <= mnPri) {
			mnPri = tokens[i].priority;
			pos = i;
		}
	}
	Assert(pos != -1, "No meaningful operator");
	*success = true;
	uint32_t left = l <= pos - 1 ? evalWithToken(l, pos - 1, success) : 0;
	if (*success == false)
		return 0;
	//Log("Pass left");
	uint32_t right =
		pos + 1 <= r ? evalWithToken(pos + 1, r, success) : 0;
	if (*success == false)
		return 0;
	//Log("Pass right");
	switch (tokens[pos].type) {
	case '+':
		*success = true;
		return left + right;
	case '-':
		*success = true;
		return left - right;
	case '*':
		*success = true;
		return left * right;
	case '/':
		// Not use negtive number
		if (right == 0) {
			Log("divide by 0");
			*success = false;
			return 0;
		}
		*success = true;
		return left / right;
	case TK_EQ:
		*success = true;
		return left == right;
	case TK_NEQ:
		*success = true;
		return left != right;
	case TK_AND:
		*success = true;
		return left && right;
	case TK_NEG:
		*success = true;
		return -right;
	case TK_POINT:
		*success = true;
		return vaddr_read(right, 4);
	}
	panic("Expression evaluating failed");
}

//#define MY_DEBUG
uint32_t expr(char *e, bool * success)
{
	if (!make_token(e)) {
		*success = false;
		return 0;
	}
//Log("Token finished");

#ifdef MY_DEBUG
	for (int i = 0; i < nr_token; i++) {
		printf("%d: %d\n", i, tokens[i].type);
		printf("%s\n", tokens[i].str);
		printf("isOp: %d Pri:%d Data:%u\n", tokens[i].isOp,
		       tokens[i].priority, tokens[i].data);
	}
#endif

	/* TODO: Insert codes to evaluate the expression. */
	//TODO();
	uint32_t ans = evalWithToken(0, nr_token - 1, success);
#ifdef MY_DEBUG
	printf("%s : %u\n", e, ans);
#endif
	return ans;
}

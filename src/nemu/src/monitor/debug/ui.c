#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
WP *createWP();
void removeWP(int no);
WP *getHeadWP();

void cpu_exec(uint64_t);

void fc2red()
{
	printf("\33[1;31m");
}

void fc2green()
{
	printf("\33[1;32m");
}

void fc2yellow()
{
	printf("\33[1;33m");
}

void fc2blue()
{
	printf("\33[1;34m");
}

void fc2purple()
{
	printf("\33[1;35m");
}

void csClear()
{
	printf("\33[0m");
}


/* We use the `readline' library to provide more flexibility to read from stdin. */
char *rl_gets()
{
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("» " /*"(nemu) " */ );

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args)
{
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args)
{
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct
{
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table[] = {
	{
	"help", "Display informations about all supported commands",cmd_help}, {
	"c", "Continue the execution of the program", cmd_c}, {
	"q", "Exit NEMU", cmd_q}, {
	"si", "Step an instruction", cmd_si}, {
	"info", "Print the state of the program", cmd_info}, {
	"x", "Get the value at address", cmd_x}, {
	"p", "Evaluate the expression", cmd_p}, {
	"w", "Create a watch point", cmd_w}, {
	"d", "Delete a watch point", cmd_d},
		/* TODO: Add more commands */
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args)
{
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if (arg == NULL) {
		/* no argument given */
		for (i = 0; i < NR_CMD; i++) {
			fc2yellow();
			printf("%-6s", cmd_table[i].name);
			csClear();
			printf(" %s\n", cmd_table[i].description);
		}
	}
	else {
		for (i = 0; i < NR_CMD; i++) {
			if (strcmp(arg, cmd_table[i].name) == 0) {
				fc2yellow();
				printf("%s", cmd_table[i].name);
				csClear();
				printf(" %s\n", cmd_table[i].description);
				return 0;
			}
		}
		WarningN("Unknown command '%s'\n", arg);
	}
	return 0;
}

static int cmd_si(char *args)
{
	char *arg = strtok(NULL, " ");
	int N;

	if (arg == NULL)
		N = 1;
	else
		N = atoi(arg);
	if (N <= 0) {
		ErrorN("Invalid step number");
		return 1;
	}

	cpu_exec(N);
	return 0;
}

static int cmd_info(char *args)
{
	char *arg = strtok(NULL, " ");

	if (arg == NULL)
		arg = "r";
	Assert(arg != NULL, "Invalid info args");

	switch (arg[0]) {
	case 'r':
		printf("eax: %.8x\n", cpu.eax);
		printf("ebx: %.8x\n", cpu.ebx);
		printf("ecx: %.8x\n", cpu.ecx);
		printf("edx: %.8x\n", cpu.edx);
		printf("esp: %.8x\n", cpu.esp);
		printf("ebp: %.8x\n", cpu.ebp);
		printf("esi: %.8x\n", cpu.esi);
		printf("edi: %.8x\n", cpu.edi);
		printf("EIP: %.8x\n", cpu.eip);
		break;
	case 'w':
		{
			//puts("Current watchpoints: ");
			for (WP * cur = getHeadWP(); cur != NULL;
			     cur = cur->next) {
				fc2blue();
				printf("%-3d", cur->NO);
				csClear();
				printf(" %s", cur->expr);
				fc2yellow();
				printf(" = %-10u\n", cur->lastVal);
				csClear();
			}
		}
		break;
	}
	return 0;
}

static int cmd_x(char *args)
{
	int N, addr;
	char *arg = strtok(NULL, " ");
	if (arg == NULL || sscanf(arg, "%d", &N) != 1) {
		ErrorN("Invalir args");
		return 1;
	}
	arg = strtok(NULL, " ");
	// Evaluate the expression
	if (arg == NULL) {
		ErrorN("Invalid args");
		return 1;
	}
	if (sscanf(arg, "%x", &addr) == 1) {

	}
	else {
		bool success = false;
		addr = expr(arg, &success);
		if (!success) {
			ErrorN("Not an valid expr.");
			return 1;
		}
		//Info("Addr: %u",addr);
	}

	uint8_t *pnt = pmem + addr;
	fc2yellow();
	printf("(%.8x)", addr);
	csClear();
	for (int i = 0; i < N; i++) {
		printf(" %.2x", *(pnt + i));
	}
	puts("");
	return 0;
}

static int cmd_p(char *args)
{
	if (args == NULL) {
		ErrorN("Invalid expression");
		return 1;
	}
	// Evaluate the expression
	bool success = false;
	uint32_t ans = expr(args, &success);
	if (!success) {
		ErrorN("Invalid expression");
		return 1;
	}
	printf("%u", ans);
	puts("");

	return 0;
}

static int cmd_w(char *args)
{
	if (args == NULL) {
		ErrorN("Invalid expression");
		return 1;
	}
	bool success = false;
	uint32_t ans = expr(args, &success);
	if (!success) {
		ErrorN("Invalid expression");
		return 1;
	}

	WP *cw = createWP();
	cw->lastVal = ans;
	strcpy(cw->expr, args);
	fc2blue();
	printf("Watchpoint created : id = %d \n", cw->NO);
	csClear();
	printf("Expr: %s\n", args);
	fc2yellow();
	printf("Last Value: %u\n", cw->lastVal);
	csClear();
	return 0;
}

static int cmd_d(char *args)
{
	int no;
	if (args == NULL || sscanf(args, "%d", &no) != 1) {
		ErrorN("Invalid args");
		return 1;
	}
	removeWP(no);
	return 0;
}


void ui_mainloop(int is_batch_mode)
{
	//static char lastStr[100];

	if (is_batch_mode) {
		cmd_c(NULL);
		return;
	}

	while (1) {
		char *str = rl_gets();

//              printf("Command: %d\n",str[0]);
/*
		if(str[0] == 0) {
			str = lastStr;
			Log("Use last command: %s",str);
			//printf("\033[1A");
			//printf("\033[7D       \033[7D");
		}

    strcpy(lastStr,str);*/
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if (cmd == NULL) {
			continue;
		}

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if (args >= str_end) {
			args = NULL;
		}

#ifdef HAS_IOE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for (i = 0; i < NR_CMD; i++) {
			if (strcmp(cmd, cmd_table[i].name) == 0) {
				if (cmd_table[i].handler(args) < 0) {
					return;
				}
				break;
			}
		}

		if (i == NR_CMD) {
			printf("Unknown command '%s'\n", cmd);
		}
	}
}

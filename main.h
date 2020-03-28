// ABGR
#define BACK_COLOR 0xFF3F0000 // Dark Blue
#define MAIN_COLOR 0xFFFFFFFF // White
#define SKIP_COLOR 0xFF7F7F7F // Semi Gray
#define WARN_COLOR 0xFF0000FF // Red
#define CHAR_HEIGHT 24

// MENU
int title_pos_x = 2;
int title_pos_y = 2;
char title[][128] = {"Naked Clusters v0.1 by Yoti", "TEST VERSION"};
int title_count = sizeof(title) / sizeof(title[0]);

int menu_pos_x = 2;
int menu_pos_y = -1;
char menu[][128] = {
	"Exit",
	"Test your ux0 partition",
	"Test your uma0 partition",
	"Test your imc0 partition",
	"Save last test results"
};
int menu_colors[] = {
	MAIN_COLOR,
	MAIN_COLOR,
	MAIN_COLOR,
	MAIN_COLOR,
	SKIP_COLOR
};
int menu_count = sizeof(menu) / sizeof(menu[0]);

//int cursor_pos_x = title_pos_x; // курсор встроен в меню
int cursor_pos_y = -1;

int info_pos_x = 2;
int info_pos_y = -1;
char info[][128] = {
	"SEE YOU SPACE COWBOY...",
	"Test speed of your memory device mounted as ux0",
	"Test speed of your memory device mounted as uma0",
	"Test speed of your memory device mounted as imc0",
	"Save your last test results as ur0:temp/last_results.txt"
};
//int info_count = sizeof(info) / sizeof(info[0]); // вывод по одной строке за раз

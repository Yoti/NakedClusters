#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/devctl.h> 
#include <psp2/rtc.h>

#include <vitasdk.h>
#include <vita2d.h>
#include "main.h"

vita2d_pgf *pgf;
vita2d_texture *bg;

uint64_t SpeedTest(char*device, int cluster) {
	void*buf = malloc(128 * 1000 * 1000 / cluster);
	uint32_t count = sizeof(buf);
	memset(buf, 0xff, count); // TODO: random

	SceRtcTick prev;
	SceRtcTick next;
	uint64_t speed_ops = 0;

	char file[128] = "";
	sprintf(file, "%s:%s", device, "naked.tmp");

	SceUID fd;
	fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT, 0777);
	uint32_t i;
	if (fd < 0) {
		free(buf);
		return fd;
	} else {
		for (i = 0; i < count; i++) {
			sceRtcGetCurrentTick(&prev);
			sceIoWrite(fd, buf, count);
			sceRtcGetCurrentTick(&next);
			speed_ops = speed_ops + (next.tick - prev.tick);
		}
	}

	sceIoClose(fd);
	sceIoRemove(file);
	free(buf);

	speed_ops = speed_ops / count;
	return speed_ops;
}

void draw_menu() {
	uint64_t result = 0;

	// правки переменных
	menu_pos_y = title_pos_y + title_count + 1;
	cursor_pos_y = menu_pos_y;
	info_pos_y = menu_pos_y + menu_count + 1;

	// проверка разделов -- cluster_size, free_size
	SceIoDevInfo info_ux0;
	if (sceIoDevctl("ux0:", 0x3001, NULL, 0, &info_ux0, sizeof(info_ux0)) < 0) {
		sprintf(info[1], "%s (not exists)", info[1]);
		menu_colors[1] = SKIP_COLOR;
	} else if (info_ux0.free_size < 150 * 1000 * 1000) {
		sprintf(info[1], "There is not enough free space on the ux0 (need 150 MB or more)");
		menu_colors[1] = SKIP_COLOR;		
	} else sprintf(info[1], "%s (%lld bytes free)", info[1], info_ux0.free_size);
	SceIoDevInfo info_uma0;
	if (sceIoDevctl("uma0:", 0x3001, NULL, 0, &info_uma0, sizeof(info_uma0)) < 0) {
		sprintf(info[2], "%s (not exists)", info[2]);
		menu_colors[2] = SKIP_COLOR;
	} else if (info_uma0.free_size < 150 * 1000 * 1000) {
		sprintf(info[2], "There is not enough free space on the uma0 (need 150 MB or more)");
		menu_colors[2] = SKIP_COLOR;		
	} else sprintf(info[2], "%s (%lld bytes free)", info[2], info_uma0.free_size);
	SceIoDevInfo info_imc0;
	if (sceIoDevctl("imc0:", 0x3001, NULL, 0, &info_imc0, sizeof(info_imc0)) < 0) {
		sprintf(info[3], "%s (not exists)", info[3]);
		menu_colors[3] = SKIP_COLOR;
	} else if (info_imc0.free_size < 150 * 1000 * 1000) {
		sprintf(info[3], "There is not enough free space on the imc0 (need 150 MB or more)");
		menu_colors[3] = SKIP_COLOR;		
	} else sprintf(info[3], "%s (%lld bytes free)", info[3], info_imc0.free_size);

	// блок инициализации управления
	SceCtrlData pad;
	memset(&pad, 0, sizeof(pad));

	while(1) {
		sceCtrlPeekBufferPositive(0, &pad, 1);
		vita2d_start_drawing();
		vita2d_clear_screen();
		vita2d_draw_texture(bg, 0, 0);

		// рисуем заголовок
		vita2d_pgf_draw_text(pgf, title_pos_x * CHAR_HEIGHT, title_pos_y * CHAR_HEIGHT + (0 * CHAR_HEIGHT), MAIN_COLOR, 1.25f, title[0]);
		vita2d_pgf_draw_text(pgf, title_pos_x * CHAR_HEIGHT, title_pos_y * CHAR_HEIGHT + (1 * CHAR_HEIGHT), WARN_COLOR, 1.0f, title[1]);

		// рисуем меню
		for (int i = 0; i < menu_count; i++) {
			if (i == cursor_pos_y - menu_pos_y)
				vita2d_pgf_draw_textf(pgf, menu_pos_x * CHAR_HEIGHT, menu_pos_y * CHAR_HEIGHT + (i * CHAR_HEIGHT), menu_colors[i], 1.0f, "> %s", menu[i]);
			else
				vita2d_pgf_draw_textf(pgf, menu_pos_x * CHAR_HEIGHT, menu_pos_y * CHAR_HEIGHT + (i * CHAR_HEIGHT), menu_colors[i], 1.0f, "%s", menu[i]);
		}

		// рисуем описание
		vita2d_pgf_draw_textf(pgf, info_pos_x * CHAR_HEIGHT, info_pos_y * CHAR_HEIGHT, MAIN_COLOR, 1.0f, "%s", info[cursor_pos_y - menu_pos_y]);

		// обрабатываем кнопки
		if (pad.buttons & SCE_CTRL_DOWN) { // пункт ниже
			if (cursor_pos_y < menu_pos_y + menu_count - 1)
				cursor_pos_y++;
			else
				cursor_pos_y = menu_pos_y;
		}
		else if (pad.buttons & SCE_CTRL_UP) { // пункт выше
			if (cursor_pos_y > menu_pos_y)
				cursor_pos_y--;
			else
				cursor_pos_y = menu_pos_y + menu_count - 1;
		}
		else if (pad.buttons & SCE_CTRL_CIRCLE) { // выход
			break;
		}
		else if (pad.buttons & SCE_CTRL_CROSS) { // выбор
			if (cursor_pos_y - menu_pos_y == 0) {
				break;
			} else if (cursor_pos_y - menu_pos_y == 1) { // ux0
				if (menu_colors[1] == MAIN_COLOR) { // пункт активен
					result = SpeedTest("ux0", info_ux0.cluster_size / 1024 * 1000);
					sprintf(title[1], "Your score for the ux0 is the %lld ops", result);
					//menu_colors[4] = MAIN_COLOR;
					sceKernelDelayThread(3.00 * 1000 * 1000);
				}
			} else if (cursor_pos_y - menu_pos_y == 2) { // uma0
				if (menu_colors[2] == MAIN_COLOR) { // пункт активен
					result = SpeedTest("uma0", info_uma0.cluster_size / 1024 * 1000);
					sprintf(title[1], "Your score for the uma0 is the %lld ops", result);
					//menu_colors[4] = MAIN_COLOR;
					sceKernelDelayThread(3.00 * 1000 * 1000);
				}
			} else if (cursor_pos_y - menu_pos_y == 3) { // imc0
				if (menu_colors[3] == MAIN_COLOR) { // пункт активен
					result = SpeedTest("imc0", info_imc0.cluster_size / 1024 * 1000);
					sprintf(title[1], "Your score for the imc0 is the %lld ops", result);
					//menu_colors[4] = MAIN_COLOR;
					sceKernelDelayThread(3.00 * 1000 * 1000);
				}
			} else if (cursor_pos_y - menu_pos_y == 4) { // dump to txt
				if (menu_colors[4] == MAIN_COLOR) { // пункт активен
					sceKernelDelayThread(3.00 * 1000 * 1000);
				}
			}
		}

		vita2d_end_drawing();
		vita2d_swap_buffers();
		sceKernelDelayThread(0.15 * 1000 * 1000);
	}
}

int main() {
	// блок инициализации libvita2d
	vita2d_init();
	vita2d_set_clear_color(BACK_COLOR); // ABGR
	pgf = vita2d_load_default_pgf();
	bg = vita2d_load_PNG_file("app0:/sce_sys/pic0.png");

	// отрисовываем меню
	sceShellUtilInitEvents(0);
	sceShellUtilLock(0x0FFF);
	draw_menu();

	// выход из меню, завершаем libvita2d
	vita2d_wait_rendering_done();
	vita2d_free_texture(bg);
	vita2d_free_pgf(pgf);
	vita2d_fini();

	// завершение работы программы
	sceShellUtilUnlock(0x0FFF);
	sceKernelExitProcess(0);
}

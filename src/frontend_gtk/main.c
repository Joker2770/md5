/*************************************************************************
    > File Name: main.c
    > Author: Jintao Yang
    > Mail: 18608842770@163.com 
    > Created Time: Sat Jan 14 09:35:20 2023
 ************************************************************************/

/*
    A simple tool for calculating MD5.
    Copyright (C) 2023  joker2770

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __linux
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif // __linux
#ifdef _MSC_VER
#include <windows.h>
#endif //_MSC_VER

#include <gtk/gtk.h>
#include "md5.h"

#define MAX_BUF_LEN (1024 * 1024)

size_t
f_size(FILE *fp)
{
  size_t n;
  fpos_t fpos;

  if (fp == NULL)
    return -1;

  fgetpos(fp, &fpos);
  fseek(fp, 0, SEEK_END);
  n = ftell(fp);
  fsetpos(fp, &fpos);

  return n;
}

int calc_md5_s(const char *src,
               char *dest)
{
  int read_len = 0;
  int i = 0;
  char temp[8] = {0};
  unsigned char decrypt[16] = {0};
  MD5_CTX md5c;

  read_len = strlen(src);

  MD5Init(&md5c);
  MD5Update(&md5c, (unsigned char *)src, read_len);
  MD5Final(&md5c, decrypt);

  for (i = 0; i < 16; i++)
  {
    sprintf(temp, "%02x", decrypt[i]);
    if (NULL != dest)
    {
      strcat((char *)dest, temp);
    }
  }
  // printf("md5:%s\n", dest);

  return read_len;
}

int calc_md5_f(const char *filename,
               size_t bf_size,
               char *dest)
{
  int i = 0;
  size_t filelen = 0;
  size_t read_len = 0;
  char temp[8] = {0};
  unsigned char *buf = NULL;
  buf = (unsigned char *)malloc(sizeof(unsigned char) * bf_size);
  unsigned char decrypt[16] = {0};
  MD5_CTX md5;
  size_t lfsize = 0;
  FILE *fdf = NULL;

  fdf = fopen(filename, "rb");
  if (NULL == fdf)
  {
    printf("%s not exist\n", filename);
    if (NULL != buf)
    {
      free(buf);
      buf = NULL;
    }
    return -1;
  }

  lfsize = f_size(fdf);
  if (lfsize <= 0)
  {
    printf("Failed to count file size!\n");
    fclose(fdf);
    if (NULL != buf)
    {
      free(buf);
      buf = NULL;
    }
    return -2;
  }
  else
    printf("file size: %zu bytes\n", lfsize);

  MD5Init(&md5);
  while (1)
  {
    memset(buf, 0, sizeof(unsigned char) * bf_size);
    read_len = fread(buf, sizeof(unsigned char), sizeof(unsigned char) * bf_size, fdf);
    if (read_len < 0)
    {
      fclose(fdf);
      if (NULL != buf)
      {
        free(buf);
        buf = NULL;
      }
      return -1;
    }
    if (read_len == 0)
      break;

    filelen += read_len;
    MD5Update(&md5, (unsigned char *)buf, read_len);

    // printf("[TIME USED = %6.2f MINUTES] [%zu%%]\r", (double)clock() / CLOCKS_PER_SEC / 60, filelen * 100 / lfsize);
    fflush(stdout);
  }
  printf("\n");

  MD5Final(&md5, decrypt);

  for (i = 0; i < 16; i++)
  {
    sprintf(temp, "%02x", decrypt[i]);
    if (NULL != dest)
    {
      strcat((char *)dest, temp);
    }
  }

  // printf("md5:%s len=%d\n", dest, filelen);
  fclose(fdf);
  if (NULL != buf)
  {
    free(buf);
    buf = NULL;
  }

  return filelen;
}

static void
activate(GtkApplication *app,
         gpointer user_data)
{
  GtkWidget *window;
  GtkWidget *content_area;
  GtkWidget *grid;
  GtkWidget *box;
  GtkWidget *text_view;
  GtkWidget *btn;
  GtkWidget *btn_file_choose;
  GtkWidget *toggle;
  GtkWidget *progress_bar;
  GtkWidget *label;
  GtkWidget *scrolled_window;
  GtkTextBuffer *gtb;

  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "md5_gtk");
  gtk_window_set_default_size(GTK_WINDOW(window), 600, 250);

  grid = gtk_grid_new();
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
  box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_set_homogeneous(GTK_BOX(box), TRUE);

  gtb = gtk_text_buffer_new(NULL);

  text_view = gtk_text_view_new();
  gtk_text_view_set_buffer(GTK_TEXT_VIEW(text_view), GTK_TEXT_BUFFER(gtb));
  scrolled_window = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scrolled_window), text_view);
  gtk_grid_attach(GTK_GRID(grid), scrolled_window, 0, 0, 3, 1);

  btn_file_choose = gtk_file_chooser_button_new("choose file", GTK_FILE_CHOOSER_ACTION_OPEN);
  gtk_widget_set_valign(btn_file_choose, GTK_ALIGN_CENTER);
  gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(btn_file_choose), TRUE);
  gtk_box_pack_start(GTK_BOX(box), btn_file_choose, FALSE, FALSE, 5);

  toggle = gtk_check_button_new_with_label("file mode");
  gtk_widget_set_valign(toggle, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(box), toggle, FALSE, FALSE, 5);

  btn = gtk_button_new_with_label("calculate");
  gtk_widget_set_valign(btn, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(box), btn, FALSE, FALSE, 5);

  gtk_grid_attach(GTK_GRID(grid), box, 0, 1, 3, 1);

  progress_bar = gtk_progress_bar_new();
  gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress_bar), TRUE);
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress_bar), 0.10);
  gtk_widget_set_sensitive(progress_bar, FALSE);
  gtk_widget_set_visible(progress_bar, FALSE);
  gtk_grid_attach(GTK_GRID(grid), progress_bar, 0, 2, 3, 1);

  label = gtk_label_new("                ");
  gtk_label_set_markup(GTK_LABEL(label), "<span font_desc=\"14.0\" weight=\"bold\" color=\"blue\" background=\"#BBFFFF\">1234567890</span>");
  gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
  gtk_label_set_selectable(GTK_LABEL(label), TRUE);
  gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 3, 1);

  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_container_add(GTK_CONTAINER(window), grid);

  gtk_window_present(GTK_WINDOW(window));
  gtk_widget_show_all(window);
  gtk_widget_set_visible(progress_bar, FALSE);
}

int
main(int argc, char *argv[])
{
  GtkApplication *app;
  int status;

  app = gtk_application_new ("com.github.md5", G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}

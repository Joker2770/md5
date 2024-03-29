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

#include <stdio.h>
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

GtkTextBuffer *gtb = NULL;
GtkWidget *window = NULL;
GtkWidget *btn = NULL;
GtkWidget *btn_file_choose = NULL;
GtkWidget *progress_bar = NULL;
GtkWidget *toggle = NULL;
GtkWidget *label = NULL;
gchar g_text[1024 * 4 + 1] = "\0";
gchar g_dest[32 + 1] = "\0";
gdouble g_fraction = 0.0;

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

int calc_md5_s(const char *src, unsigned int src_len,
               char *dest, unsigned int dst_len)
{
  int i = 0;
  char temp[8] = {0};
  unsigned char decrypt[16] = {0};
  MD5_CTX md5c;

  MD5Init(&md5c);
  MD5Update(&md5c, (unsigned char *)src, src_len);
  MD5Final(&md5c, decrypt);

  if (NULL != dest)
  {
    for (i = 0; i < 16; i++)
    {
      sprintf(temp, "%02x", decrypt[i]);
      strcat((char *)dest, temp);
    }
  }
  // printf("md5:%s\n", dest);

  return strlen(decrypt);
}

int calc_md5_f(const char *filename,
               size_t bf_size,
               char *dest)
{
  int i = 0;
  size_t filelen = 0;
  char temp[8] = {0};
  unsigned char *buf = NULL;
  buf = (unsigned char *)malloc(sizeof(unsigned char) * bf_size);
  if (NULL == buf)
    return -1;
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
  if (lfsize == 0)
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
    size_t read_len = 0;
    memset(buf, 0, sizeof(unsigned char) * bf_size);
    read_len = fread(buf, sizeof(unsigned char), sizeof(unsigned char) * bf_size, fdf);
    // if (read_len < 0)
    // {
    //   fclose(fdf);
    //   if (NULL != buf)
    //   {
    //     free(buf);
    //     buf = NULL;
    //   }
    //   return -1;
    // }
    if (read_len == 0)
      break;

    filelen += read_len;
    MD5Update(&md5, (unsigned char *)buf, read_len);

    // printf("[TIME USED = %6.2f MINUTES] [%zu%%]\r", (double)clock() / CLOCKS_PER_SEC / 60, filelen * 100 / lfsize);
    g_fraction = (double)filelen / (double)lfsize;

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
show_errMsg(const gchar* errMsg, gpointer data)
{
		GtkWidget *dialog = NULL;
		dialog = gtk_message_dialog_new(GTK_WINDOW(data),
					GTK_DIALOG_DESTROY_WITH_PARENT,
					GTK_MESSAGE_ERROR,
					GTK_BUTTONS_OK,
					"%s",errMsg);
		gtk_window_set_title(GTK_WINDOW(dialog), "Error");
		gtk_widget_show_all (dialog);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
}

gboolean timeout_callback(gpointer data)
{
  if (NULL != data)
  {
    g_print("%f\n", g_fraction);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data), g_fraction);
  }
  return g_fraction < 1.0;
}

gboolean calc_file_thread(void *arg)
{
  memset(g_dest, 0, sizeof(g_dest));
  calc_md5_f((const char *)arg, MAX_BUF_LEN, g_dest);
  if (NULL != label)
  {
    gchar szMarkup[128] = "\0";
    sprintf(szMarkup, "<span font_desc=\"14.0\" weight=\"bold\" color=\"blue\">%s</span>", g_dest);
    gtk_label_set_markup(GTK_LABEL(label), szMarkup);
  }

  gtk_widget_set_sensitive(btn, TRUE);

  return TRUE;
}

static void
gtb_chg_callback(GtkWidget *widget, gpointer data)
{
  gchar *text = NULL;
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(widget), &start, &end);

  const GtkTextIter s = start, e = end;
  text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(widget), &s, &e, TRUE);

  g_print("text view(send):%s\n", text);

  if (strlen(text) < (1024 * 4))
  {
    memset(g_text, 0, sizeof(g_text));
    memcpy(g_text, (gchar *)text, strlen(text) * sizeof(gchar));
  }
  g_free(text);
  text = NULL;
}

static void
calc_md5(GtkWidget *widget,
             gpointer data)
{
  gchar szDest[32 + 1] = "\0";
  memset(szDest, 0, sizeof(szDest));
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle)))
  {
    gchar* szFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(btn_file_choose));
    if (NULL != szFileName)
    {
      gtk_widget_set_visible(progress_bar, TRUE);
      gtk_widget_set_sensitive(btn, FALSE);
      GThread *thread = NULL;
      thread = g_thread_new(NULL, (GThreadFunc)calc_file_thread, (gpointer)szFileName);
      g_timeout_add(100, (GSourceFunc)timeout_callback, (gpointer)progress_bar);

      if (NULL != thread)
      {
        g_thread_unref(thread);
      }
    }
    else
      show_errMsg("Please choose a file first!", (gpointer)window);
  }
  else
  {
    gtk_widget_set_visible(progress_bar, FALSE);
    calc_md5_s(g_text, strlen(g_text), szDest, 32);
    if (NULL != data)
    {
      gchar szMarkup[128] = "\0";
      sprintf(szMarkup, "<span font_desc=\"14.0\" weight=\"bold\" color=\"blue\">%s</span>", szDest);
      gtk_label_set_markup(GTK_LABEL(data), szMarkup);
    }
  }
}

static void
activate(GtkApplication *app,
         gpointer user_data)
{
  GtkWidget *content_area = NULL;
  GtkWidget *grid = NULL;
  GtkWidget *box = NULL;
  GtkWidget *text_view = NULL;
  GtkWidget *scrolled_window = NULL;

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
	g_signal_connect(gtb, "changed", G_CALLBACK(gtb_chg_callback), NULL);

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
  gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress_bar), 0.01);
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress_bar));
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
  gtk_widget_set_sensitive(progress_bar, FALSE);
  gtk_widget_set_visible(progress_bar, FALSE);
  gtk_grid_attach(GTK_GRID(grid), progress_bar, 0, 2, 3, 1);

  label = gtk_label_new("                ");
  gtk_label_set_single_line_mode(GTK_LABEL(label), TRUE);
  gtk_label_set_selectable(GTK_LABEL(label), TRUE);
  gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 3, 1);

  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_container_add(GTK_CONTAINER(window), grid);

  g_signal_connect(btn, "clicked", G_CALLBACK(calc_md5), (gpointer)label);

  gtk_window_present(GTK_WINDOW(window));
  gtk_widget_show_all(window);
  gtk_widget_set_visible(progress_bar, FALSE);
}

int
main(int argc, char *argv[])
{
  GtkApplication *app;
  int status;

  app = gtk_application_new (NULL, G_APPLICATION_FLAGS_NONE);
  g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
  status = g_application_run (G_APPLICATION (app), argc, argv);
  g_object_unref (app);

  return status;
}

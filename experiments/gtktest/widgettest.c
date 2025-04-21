//
// BSD 2-Clause License
//
// Copyright (c) 2024, dettus@dettus.net
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "dMagnetic2_errorcodes.h"
#include "dMagnetic2_graphics.h"        // for the xpm function
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#define	MAX_WIDTH	640
#define	MAX_HEIGHT	480

typedef struct _tGUIHandle
{
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *button;
	GtkWidget *picture;
	GdkPixbuf *pixbuf_ping;
	GdkPixbuf *pixbuf_pong;
	GtkWidget *scrollview;
	GtkTextBuffer *textbuffer;
	GtkWidget *textview;
	unsigned char drawbuf[4*MAX_WIDTH*MAX_HEIGHT];	// red, green, blue, alpha
	GdkTexture *texture_ping;
	GdkTexture *texture_pong;
	int ping0_pong1;
	pthread_mutex_t mutex;

} tGUIHandle;

typedef struct _tdMagneticHandle
{
	void *pGfxHandle;
	int gfxhandlesize;
	void *pTmpBuf;
	int tmpsize;
	unsigned char pGfxBuf[1<<22];
	int gfxsize;
	int picnum;
	tdMagnetic2_canvas_small canvas_small;
} tdMagneticHandle;

typedef struct _tHandle
{
	tGUIHandle		hGui;
	tdMagneticHandle	hdMagnetic;
} tHandle;


int dMagnetic_start(void* handle)
{
	tdMagneticHandle* pThis=(tdMagneticHandle*)handle;
	int retval;

	retval=dMagnetic2_graphics_getsize(&(pThis->gfxhandlesize),&(pThis->tmpsize));
	if (retval==DMAGNETIC2_OK)
	{
		pThis->pGfxHandle=malloc(pThis->gfxhandlesize);
		pThis->pTmpBuf=malloc(pThis->tmpsize);

		memset(pThis->pGfxHandle,0,pThis->gfxhandlesize);
		memset(pThis->pTmpBuf,0,pThis->tmpsize);

		retval=dMagnetic2_graphics_init(pThis->pGfxHandle,pThis->pTmpBuf);
	}
	return retval;	
}

int dMagnetic_setgfx(void *handle,char* filename)
{
	FILE *f;
	tdMagneticHandle* pThis=(tdMagneticHandle*)handle;
	int retval;

	f=fopen(filename,"rb");
	if (!f)
	{
		return -10;
	}
	pThis->gfxsize=fread(pThis->pGfxBuf,sizeof(char),sizeof(pThis->pGfxBuf),f);
	fclose(f);
	printf("loaded %d bytes\n",pThis->gfxsize);
	
	retval=dMagnetic2_graphics_set_gfx(pThis->pGfxHandle,pThis->pGfxBuf,pThis->gfxsize);
	return retval;	
}
static void gui_textview_scrolldown(GtkWidget *widget,gpointer user_data)
{
	int lastline;
	GtkTextIter iter;
	tHandle* pThis=(tHandle*)user_data;
	tGUIHandle* pGUI=(tGUIHandle*)&(pThis->hGui);
	tdMagneticHandle* pdMagnetic=(tdMagneticHandle*)&(pThis->hdMagnetic);

	lastline=gtk_text_buffer_get_line_count(pGUI->textbuffer);
	gtk_text_buffer_get_iter_at_line(pGUI->textbuffer,&iter,lastline);
	gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(pGUI->textview),&iter,0,false,0,0);
	

// https://docs.gtk.org/gtk4/section-text-widget.html
//// Use a tag to change the color for just one part of the widget 
//tag = gtk_text_buffer_create_tag (buffer, "blue_foreground",
//                                  "foreground", "blue",
//                                  NULL);
//gtk_text_buffer_get_iter_at_offset (buffer, &start, 7);
//gtk_text_buffer_get_iter_at_offset (buffer, &end, 12);
//gtk_text_buffer_apply_tag (buffer, tag, &start, &end);

}
static void gui_next_clicked(GtkWidget *widget,gpointer user_data)
{
	tHandle* pThis=(tHandle*)user_data;
	tGUIHandle* pGUI=(tGUIHandle*)&(pThis->hGui);
	tdMagneticHandle* pdMagnetic=(tdMagneticHandle*)&(pThis->hdMagnetic);
	int retval;
	char tmp[32];

	pthread_mutex_lock(&(pGUI->mutex));
	retval=dMagnetic2_graphics_decode_by_picnum(pdMagnetic->pGfxHandle,pdMagnetic->picnum,&(pdMagnetic->canvas_small),NULL);
	printf("%2d> retval:%d width:%d height:%d\n",pdMagnetic->picnum,retval,pdMagnetic->canvas_small.width,pdMagnetic->canvas_small.height);
	if (retval==DMAGNETIC2_OK && pdMagnetic->canvas_small.width>0 && pdMagnetic->canvas_small.height>0)
	{
		int width;
		int height;
		GdkPixbuf *pixbuf;
		retval=dMagnetic2_graphics_canvas_small_to_8bit(&(pdMagnetic->canvas_small),true,pGUI->drawbuf,&width,&height);
	
		pixbuf=gdk_pixbuf_new_from_data(pGUI->drawbuf,GDK_COLORSPACE_RGB,true,8,width,height,width*4,NULL,NULL);
		if (!pGUI->ping0_pong1)
		{
			pGUI->pixbuf_pong=gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,width,height);
			gdk_pixbuf_copy_area(pixbuf,0,0,width,height,pGUI->pixbuf_pong,0,0);
			pGUI->texture_pong=gdk_texture_new_for_pixbuf(pGUI->pixbuf_pong);
			gtk_picture_set_paintable(GTK_PICTURE(pGUI->picture),GDK_PAINTABLE(pGUI->texture_pong));
			
			g_object_unref(pGUI->pixbuf_ping);
			g_object_unref(pGUI->texture_ping);
			pGUI->ping0_pong1=1;
		} else {
			pGUI->pixbuf_ping=gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,width,height);
			gdk_pixbuf_copy_area(pixbuf,0,0,width,height,pGUI->pixbuf_ping,0,0);
			pGUI->texture_ping=gdk_texture_new_for_pixbuf(pGUI->pixbuf_ping);	
			gtk_picture_set_paintable(GTK_PICTURE(pGUI->picture),GDK_PAINTABLE(pGUI->texture_ping));

			g_object_unref(pGUI->pixbuf_pong);
			g_object_unref(pGUI->texture_pong);
			pGUI->ping0_pong1=0;
		}
		gtk_widget_queue_draw(pGUI->picture);
		g_object_unref(pixbuf);
	}
	pdMagnetic->picnum=(pdMagnetic->picnum+1)%32;
	snprintf(tmp,32,"PIC:%d\n",pdMagnetic->picnum);
	gtk_text_buffer_insert_at_cursor(pGUI->textbuffer,tmp,strlen(tmp));
	gtk_widget_queue_resize(pGUI->textview);
	gui_textview_scrolldown(pGUI->textview,pThis);
	pthread_mutex_unlock(&(pGUI->mutex));
}
static void gui_activate(GtkApplication* app,gpointer user_data)
{
	tHandle* pThis=(tHandle*)user_data;
	tGUIHandle* pGUI=(tGUIHandle*)&(pThis->hGui);
	tdMagneticHandle* pdMagnetic=(tdMagneticHandle*)&(pThis->hdMagnetic);
	
	pGUI->app=app;
	pGUI->window=gtk_application_window_new(pGUI->app);
	gtk_window_set_title(GTK_WINDOW(pGUI->window),"hello there!");	
	g_signal_connect(G_OBJECT(pGUI->window),"close_request",G_CALLBACK(gtk_window_destroy),pGUI->window);


	pGUI->grid=gtk_grid_new();
	
	gtk_widget_set_halign(pGUI->grid,GTK_ALIGN_BASELINE_FILL);
	gtk_widget_set_valign(pGUI->grid,GTK_ALIGN_START);
	gtk_widget_set_hexpand(pGUI->grid,true);
	gtk_widget_set_vexpand(pGUI->grid,true);
	gtk_grid_set_row_homogeneous(GTK_GRID(pGUI->grid),false);


	pGUI->button=gtk_button_new_with_label("next");
	gtk_widget_set_hexpand(pGUI->button,true);
	gtk_widget_set_vexpand(pGUI->button,false);
	g_signal_connect(pGUI->button,"clicked",G_CALLBACK(gui_next_clicked),pThis);
//	gtk_box_append(GTK_BOX(pGUI->box),pGUI->button);
	gtk_grid_attach(GTK_GRID(pGUI->grid),pGUI->button,0,0, 1,1);


	pGUI->picture=gtk_picture_new_for_paintable(NULL);
	gtk_widget_set_visible(pGUI->picture,true);
	gtk_widget_set_hexpand(pGUI->picture,true);
	gtk_widget_set_vexpand(pGUI->picture,true);

	pGUI->pixbuf_ping=gdk_pixbuf_new(GDK_COLORSPACE_RGB,true,8,640,480);
	pGUI->pixbuf_pong=NULL;
	{
		GdkPixbuf *pixbuf;
		int i;
		for (i=0;i<640*480;i++)
		{
			pGUI->drawbuf[4*i+0]=0x55;
			pGUI->drawbuf[4*i+1]=0x55;
			pGUI->drawbuf[4*i+2]=0xff;
			pGUI->drawbuf[4*i+3]=0xff;
		}
		for (i=0;i<480;i++)
		{
			pGUI->drawbuf[4*(640*i+i)+0]=0xff;
			pGUI->drawbuf[4*(640*i+i)+1]=0x55;
			pGUI->drawbuf[4*(640*i+i)+2]=0x55;
			pGUI->drawbuf[4*(640*i+i)+3]=0xff;
		}
		pixbuf=gdk_pixbuf_new_from_data(pGUI->drawbuf,GDK_COLORSPACE_RGB,TRUE,8,640,480,640*4,NULL,NULL);
		gdk_pixbuf_copy_area(pixbuf,0,0,gdk_pixbuf_get_width(pixbuf),gdk_pixbuf_get_height(pixbuf),pGUI->pixbuf_ping,0,0);	
		g_object_unref(pixbuf);
	}
	pGUI->texture_ping=gdk_texture_new_for_pixbuf(pGUI->pixbuf_ping);
	pGUI->texture_pong=NULL;
	pGUI->ping0_pong1=0;

	gtk_picture_set_paintable(GTK_PICTURE(pGUI->picture),GDK_PAINTABLE(pGUI->texture_ping));
//	gtk_box_append(GTK_BOX(pGUI->box),pGUI->picture);
	gtk_grid_attach(GTK_GRID(pGUI->grid),pGUI->picture,0,1, 1,5);
	pGUI->scrollview=gtk_scrolled_window_new();
	gtk_widget_set_halign(pGUI->scrollview,GTK_ALIGN_BASELINE_FILL);
	gtk_widget_set_valign(pGUI->scrollview,GTK_ALIGN_START);
	gtk_widget_set_hexpand(pGUI->scrollview,true);
	gtk_widget_set_vexpand(pGUI->scrollview,true);
	gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(pGUI->scrollview),480);
	gtk_scrolled_window_set_max_content_height(GTK_SCROLLED_WINDOW(pGUI->scrollview),480);
//	gtk_scrolled_window_set_hadjustment(GTK_SCROLLED_WINDOW(pGUI->scrollview),false);
//	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(pGUI->scrollview),true);
	gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(pGUI->scrollview),true);
//	gtk_box_append(GTK_BOX(pGUI->box),pGUI->scrollview);
	gtk_grid_attach(GTK_GRID(pGUI->grid),pGUI->scrollview,0,6, 1,4);

	pGUI->textview=gtk_text_view_new();
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(pGUI->textview),true);
	gtk_widget_set_halign(pGUI->textview,GTK_ALIGN_BASELINE_FILL);
	gtk_widget_set_valign(pGUI->textview,GTK_ALIGN_START);
	gtk_widget_set_hexpand(pGUI->textview,true);
	gtk_widget_set_vexpand(pGUI->textview,true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(pGUI->scrollview),pGUI->textview);

	pGUI->textbuffer=gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(pGUI->textbuffer,"Thomas Dettbarn\nHeimat\n",23);
	
	



	gtk_text_view_set_buffer(GTK_TEXT_VIEW(pGUI->textview),pGUI->textbuffer);
	gtk_window_set_child(GTK_WINDOW(pGUI->window),pGUI->grid);
	gtk_window_present(GTK_WINDOW(pGUI->window));
        pthread_mutex_init(&(pGUI->mutex),NULL);
}

int gui_run(void* GUIhandle,void *handle,int argc,char** argv)
{
	tGUIHandle* pThis=(tGUIHandle*)GUIhandle;
	int status;
	pThis->app=gtk_application_new(NULL,G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(pThis->app,"activate",G_CALLBACK(gui_activate),handle);
	status=g_application_run(G_APPLICATION(pThis->app),argc,argv);
	g_object_unref(pThis->app);
	return status;
}

int main(int argc,char** argv)
{
	int status;
	GtkApplication *app;
	void *pHandle;
	tHandle *pThis;
	int retval;

	pHandle=malloc(sizeof(tHandle));
	memset(pHandle,0,sizeof(tHandle));
	pThis=(tHandle*)pHandle;

	retval=dMagnetic_start(&(pThis->hdMagnetic));
	printf("loading guild.gfx\n");
	if (retval==DMAGNETIC2_OK)
	{
		dMagnetic_setgfx(&(pThis->hdMagnetic),"guild.gfx");	
	}
	if (retval==DMAGNETIC2_OK)
	{
		retval=gui_run(&(pThis->hGui),pHandle,argc,argv);
	}
	return retval;	
}



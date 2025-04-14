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


typedef struct _tHandle
{
	GtkApplication *app;
	GtkWidget *window;
	GtkWidget *picture;
	GdkPixbuf *pixbuf_ping;
	GdkPixbuf *pixbuf_pong;
	GdkTexture *texture_ping;
	GdkTexture *texture_pong;
	int ping0_pong1;

	GtkWidget *button;
	GtkWidget *box;
	unsigned char drawbuf[4*640*480];

	tdMagnetic2_canvas_small canvas_small;

	void *pGfxHandle;
	void *pTmpBuf;
	int picnum;
	unsigned char pGfxBuf[1<<22];

     	pthread_mutex_t mutex;
} tHandle;

void save_picture(int num,GdkPixbuf* pixbuf,tdMagnetic2_canvas_small *pSmall)
{
	char filename[32];
	GError *err;
	unsigned char *pxpm;
	FILE *f;

	snprintf(filename,32,"%08d.pixbuf.png",num);
	err=NULL;
	gdk_pixbuf_save(pixbuf,filename,"png",&err,NULL);
	pxpm=malloc(1<<20);
	dMagnetic2_graphics_canvas_small_to_xpm(pSmall,pxpm,(1<<20));
	f=fopen("test.xpm","wb");
	fprintf(f,"%s",pxpm);
	fclose(f);
	free(pxpm);
}

static gboolean heartbeat(gpointer user_data)
{
        tHandle* pThis=(tHandle*)user_data;
	pthread_mutex_lock(&(pThis->mutex));
	gtk_widget_queue_draw(pThis->window);
	pthread_mutex_unlock(&(pThis->mutex));
	return G_SOURCE_CONTINUE;
	
}

static void next_clicked(GtkWidget *widget,gpointer user_data)
{
        tHandle* pThis=(tHandle*)user_data;

	int retval;

	pthread_mutex_lock(&(pThis->mutex));
	retval=dMagnetic2_graphics_decode_by_picnum(pThis->pGfxHandle,pThis->picnum,&(pThis->canvas_small),NULL);
	if (retval==DMAGNETIC2_OK && pThis->canvas_small.width>0 && pThis->canvas_small.height>0)
	{
		int width;
		int height;
		int retval;
		GdkPixbuf *pixbuf;
		GdkPixbuf *dest_pixbuf;
		GdkTexture *dest_texture;

		retval=dMagnetic2_graphics_canvas_small_to_8bit(&(pThis->canvas_small),TRUE,pThis->drawbuf,&width,&height);


		if (pThis->picnum&1)
		{		
			dest_pixbuf=pThis->pixbuf_ping;
			dest_texture=pThis->texture_ping;
		} else {
			dest_pixbuf=pThis->pixbuf_ping;
			dest_texture=pThis->texture_pong;
		}
		pixbuf=gdk_pixbuf_new_from_data(pThis->drawbuf,
			GDK_COLORSPACE_RGB,TRUE,8,
			width,height,
			width*4,NULL,NULL);
		gdk_pixbuf_copy_area(pixbuf,0,0,gdk_pixbuf_get_width(pixbuf),gdk_pixbuf_get_height(pixbuf),dest_pixbuf,0,0);
		g_object_unref(pixbuf);
		save_picture(pThis->picnum,dest_pixbuf,&(pThis->canvas_small));

        	gtk_picture_set_paintable(GTK_PICTURE(pThis->picture),GDK_PAINTABLE(dest_texture));
		gtk_widget_queue_draw(pThis->picture);

		if (pThis->picnum&1)
		{		
//			g_object_unref(pThis->pixbuf_pong);
//			pThis->pixbuf_pong=gdk_pixbuf_new_from_data(pThis->drawbuf, GDK_COLORSPACE_RGB,TRUE,8,640,480,640*4,NULL,NULL);
			g_object_unref(pThis->texture_pong);
        		pThis->texture_pong=gdk_texture_new_for_pixbuf(pThis->pixbuf_ping);
		} else {
//			g_object_unref(pThis->pixbuf_ping);
//			pThis->pixbuf_ping=gdk_pixbuf_new_from_data(pThis->drawbuf, GDK_COLORSPACE_RGB,TRUE,8,640,480,640*4,NULL,NULL);
			g_object_unref(pThis->texture_ping);
        		pThis->texture_ping=gdk_texture_new_for_pixbuf(pThis->pixbuf_ping);
		}
	}
	pThis->picnum=(pThis->picnum+1)%32;
	pthread_mutex_unlock(&(pThis->mutex));
}

static void activate(GtkApplication* app,gpointer user_data)
{
        tHandle* pThis=(tHandle*)user_data;
        pThis->app=app;

        pThis->window=gtk_application_window_new(app);
        gtk_window_set_title(GTK_WINDOW(pThis->window),"hello there!");
        gtk_window_set_default_size(GTK_WINDOW(pThis->window),640,480);


	pThis->box=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
	gtk_widget_set_halign (pThis->box, GTK_ALIGN_CENTER);
	gtk_widget_set_valign (pThis->box, GTK_ALIGN_CENTER);

	gtk_window_set_child(GTK_WINDOW(pThis->window),pThis->box);
	pThis->button=gtk_button_new_with_label("next");
	g_signal_connect(pThis->button,"clicked",G_CALLBACK(next_clicked),pThis);
	gtk_box_append(GTK_BOX(pThis->box),pThis->button);



        pThis->picture=gtk_picture_new_for_paintable(NULL);
	gtk_widget_set_visible(pThis->picture,true);

 //       pThis->pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB,FALSE,8,640,200);
	{
		int i;
		for (i=0;i<640*480*4;i+=3) pThis->drawbuf[i]=0xff;
	}
	pThis->pixbuf_ping=gdk_pixbuf_new_from_data(pThis->drawbuf, GDK_COLORSPACE_RGB,TRUE,8,640,480,640*4,NULL,NULL);
        pThis->texture_ping=gdk_texture_new_for_pixbuf(pThis->pixbuf_ping);
	{
		int i;
		for (i=0;i<640*480*4;i+=3) pThis->drawbuf[i]=0x7f;
	}
	pThis->pixbuf_pong=gdk_pixbuf_new_from_data(pThis->drawbuf, GDK_COLORSPACE_RGB,TRUE,8,640,480,640*4,NULL,NULL);
        pThis->texture_pong=gdk_texture_new_for_pixbuf(pThis->pixbuf_pong);

        gtk_picture_set_paintable(GTK_PICTURE(pThis->picture),GDK_PAINTABLE(pThis->texture_ping));
	gtk_box_append(GTK_BOX(pThis->box),pThis->picture);

        g_signal_connect(G_OBJECT(pThis->window),"close_request",G_CALLBACK(gtk_window_destroy),pThis->window);
        gtk_window_present(GTK_WINDOW(pThis->window));

        pthread_mutex_init(&(pThis->mutex),NULL);
    	g_timeout_add(19,heartbeat,pThis);  // every 19 ms call the heartbeat
}

tHandle handle;
int main(int argc,char** argv)
{
	GtkApplication *app;
	int status;
	void *pHandle;
	int retval;
	int size_handle;
	int size_tmpbuf;
	FILE *f;
	int gfxsize;
	memset(&handle,0,sizeof(tHandle));

	retval=dMagnetic2_graphics_getsize(&size_handle,&size_tmpbuf);
	handle.pGfxHandle=malloc(size_handle);
	handle.pTmpBuf=malloc(size_tmpbuf);

	retval=dMagnetic2_graphics_init(handle.pGfxHandle,handle.pTmpBuf);
	
	f=fopen("guild.gfx","rb");
	gfxsize=fread(handle.pGfxBuf,sizeof(char),sizeof(handle.pGfxBuf),f);
	fclose(f);

	dMagnetic2_graphics_set_gfx(handle.pGfxHandle,handle.pGfxBuf,gfxsize);

	handle.picnum=0;



	app=gtk_application_new(NULL,G_APPLICATION_DEFAULT_FLAGS);
	g_signal_connect(app,"activate",G_CALLBACK(activate),&handle);
	status=g_application_run(G_APPLICATION(app),argc,argv);
	g_object_unref(app);
	return status;
}



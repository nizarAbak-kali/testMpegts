#include <stdio.h>
#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <gst/app/app.h>

static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
    GMainLoop *loop = data;

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print ("End-of-stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug = NULL;
            GError *err = NULL;

            gst_message_parse_error (msg, &err, &debug);

            g_print ("Error: %s\n", err->message);
            g_error_free (err);

            if (debug) {
                g_print ("Debug details: %s\n", debug);
                g_free (debug);
            }

            g_main_loop_quit (loop);
            break;
        }
        default:
            break;
    }

    return TRUE;
}


void test_mux(){

}

void test_demux(){

}

void test_all(){

}




gint main (gint   argc,
      gchar *argv[])
{
    GstStateChangeReturn ret;
    GstElement *pipeline, *filesrc, *h264decoder, *mts_sink;
    GstElement *queue2, *queue1, *queue3, *queue4, *fakesrc, *x264encoder;
    GstElement  *avifilesink, *undefined_sink;
    GstElement *identity, *typefind1, *typefind2;
    GstElement *demux, *mux, *videotestsrc;

    GstElement *typefind3;

    GMainLoop *loop;
    GstBus *bus;
    guint watch_id;

    /* initialization */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);
    /*if (argc != 2) {
        g_print ("Usage: %s <ts file location>\n", argv[0]);
        return 01;
    }
*/
    /* create elements */
    pipeline = gst_pipeline_new ("my_pipeline");
    gchar * ts_location = "/home/niaba/lol.mts";
    gchar * avi_location = "/home/niaba/lol.avi";
    gchar * lol_location = "/home/niaba/lol";

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    watch_id = gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);
    // partie mux

    videotestsrc = gst_element_factory_make("videotestsrc","videotest");
    g_object_set(G_OBJECT(videotestsrc), "num-buffers", 50, NULL);
    //FIXME: ask for sink and source file paths in the runtime var
    mux = gst_element_factory_make ("mpegtsmux", "my_ts_muxer");
    mts_sink = gst_element_factory_make("filesink","mts_sink");
    g_object_set(G_OBJECT(mts_sink), "location", ts_location, NULL);
    x264encoder = gst_element_factory_make ("x264enc", "264encoder");
    fakesrc = gst_element_factory_make("fakesrc","faksrc");
    g_object_set (G_OBJECT (fakesrc), "do-timestamp", TRUE, NULL);
    g_object_set (G_OBJECT (fakesrc), "blocksize", 80, NULL);
    g_object_set (G_OBJECT (fakesrc), "num-buffers", 50, NULL);
    g_object_set (G_OBJECT (fakesrc), "sizetype", 1, NULL);
    g_object_set (G_OBJECT (fakesrc), "format", 3, NULL);
    g_object_set (G_OBJECT (fakesrc), "filltype", 3, NULL);
    identity = gst_element_factory_make("identity", "identity");
    g_object_set(G_OBJECT(identity), "sync" , TRUE, NULL);
    queue1   = gst_element_factory_make("queue","queue1");
    queue2   = gst_element_factory_make("queue","queue2");

    // partie demux
    filesrc  = gst_element_factory_make ("filesrc", "my_filesource");
    g_object_set(G_OBJECT(filesrc), "location", ts_location, NULL);
    demux = gst_element_factory_make ("tsdemux", "my_ts_demuxer");
    typefind1 = gst_element_factory_make("typefind", "typefind1");
    typefind2 = gst_element_factory_make("typefind", "typefind2");
    typefind3 = gst_element_factory_make("typefind", "typefind3");
    h264decoder = gst_element_factory_make("avdec_h264", "264_dec");
    avifilesink = gst_element_factory_make("filesink", "avi_file_sink");
    g_object_set(G_OBJECT(avifilesink), "location", avi_location, NULL);
    undefined_sink = gst_element_factory_make("filesink", "unknown_sink");
    g_object_set(G_OBJECT(undefined_sink), "location", lol_location, NULL);
    queue3   = gst_element_factory_make("queue","queue3");
    queue4   = gst_element_factory_make("queue","queue4");




    if (!undefined_sink){
        g_print("undefined sink plugin didn't work ....\n");
    }
    if (!h264decoder){
        g_print("h264decoder plugin didn't work ....\n");
    }
    if (!avifilesink){
        g_print("avifilesink plugin didn't work ....\n");
    }
    if (!typefind1 || !typefind2 || !typefind3){
        g_print("typefind plugin didn't work ....\n");
    }
    if (!filesrc ) {
        g_print ("filesrc plugin didn't work ....\n");
        return -1;
    }if (!videotestsrc  ) {
        g_print ("videotestsrc plugin didn't work ....\n");
        return -1;
    }if (!mux ) {
        g_print ("mux plugin didn't work ....\n");
        return -1;
    }if (!mts_sink) {
        g_print ("mts_sink plugin didn't work ....\n");
        return -1;
    }if (!x264encoder  ) {
        g_print ("x264encoder  plugin didn't work ....\n");
        return -1;
    }if (!fakesrc) {
        g_print ("fakesrc plugin didn't work ....\n");
        return -1;
    }if (!identity) {
        g_print ("identity plugin didn't work ....\n");
        return -1;
    }if ( !queue1 || !queue2 || !queue3 || !queue4) {
        g_print ("queue plugin didn't work ....\n");
        return -1;
    }
    // add all element into the pipeline
    gst_bin_add_many(GST_BIN(pipeline), filesrc, demux, videotestsrc, mux, mts_sink,
                     x264encoder, fakesrc, identity, queue1, queue2,queue3, queue4, typefind3, undefined_sink, NULL);



    // LINKING THE MUX PART
/*
    metaklv = gst_caps_from_string("meta/x-klv");
    g_print("%s \n",gst_caps_to_string(metaklv));
    GstCaps *xprivate = gst_caps_from_string("application/x-private");
    g_print("%s ", gst_caps_from_string(xprivate));

    if(!gst_element_link_filtered(fakesrc,identity,xprivate)){
        g_print("link between fakesrc and identiy didn't work");
        return -1;
    }
    gst_caps_unref(metaklv);
*/
    GST_WARNING("LINKING MUXING PART");
    if(!gst_element_link(fakesrc,identity)){
        g_print("link between fakesrc and identiy didn't work");
        return -1;
    }

    if(!gst_element_link (identity, mux)){
        g_print("link between identity and mux didn't work");
        return -1;
    }

    if(!gst_element_link(videotestsrc,x264encoder)){
        g_print("link between videotestsrc and x264enc didn't work");
        return -1;
    }

    if(!gst_element_link(x264encoder, mux)){
        g_print("link between x264enc and mux didn't work");
        return -1;
    }


    if(!gst_element_link(mux, mts_sink)){
        g_print("link between mux and mts sink didn't work");
        return -1;
    }



    // TODO REPLACE meta/x-klv by application/x-private , pid = xx


    // LINKINK THE DEMUXING PART

    if(!gst_element_link(filesrc ,typefind3)){
        g_print("link between filesrc and identiy didn't work");
        return -1;
    }

    if(!gst_element_link (typefind3, demux)){
        g_print("link between typefind  and demux didn't work");
        return -1;
    }

    if(!gst_element_link(demux,queue3)){
        g_print("link between demux and queue3 didn't work");
        return -1;
    }

    if(!gst_element_link(demux, queue4)){
        g_print("link between demux and queue4 didn't work");
        return -1;
    }

    GstCaps *caps264 = gst_caps_from_string("video/x-h264");

    if(!gst_element_link_filtered(queue3, h264decoder , caps264)){
        g_print("link between queue3 and h264dec : video/x-h264 sink didn't work");
        return -1;
    }

    if(!gst_element_link(h264decoder, avifilesink)){
        g_print("link between h264decoder and avifilesink didn't work");
        return -1;
    }

    if(!gst_element_link(queue4, undefined_sink)){
        g_print("link between demux and queue4 didn't work");
        return -1;
    }


    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        GstMessage *msg;

        g_print ("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error (msg, &err, NULL);
            g_print ("ERROR: %s\n", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return -1;
    }
    g_print("Running...\n");
    g_main_loop_run (loop);

    /* clean up */
    g_print("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);
    
    g_print("Deleting pipeline\n");
    gst_object_unref (pipeline);
    g_source_remove (watch_id);
    g_main_loop_unref (loop);

    return 0;
}

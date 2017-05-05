#include <stdio.h>
#include <gst/gst.h>
#include <stdlib.h>
#include <string.h>


typedef struct _Test Test;

struct _Test {
    GstElement *pipeline;
    GMainLoop *loop;
    GstBus *bus;
    gchar *pipeline_name;
    guint watch_id;
    gint queue_id;
};

Test s_test;

static gboolean
bus_call(GstBus *bus,
         GstMessage *msg,
         Test *test) {

    GMainLoop *loop = test->loop;

    GST_WARNING("Got this message : %s  and this type %d",
                GST_MESSAGE_TYPE_NAME(msg), GST_MESSAGE_TYPE(msg));

    switch (GST_MESSAGE_TYPE (msg)) {
        case GST_MESSAGE_EOS:
            g_print("End-of-stream\n");
            g_main_loop_quit(loop);
            break;

        case GST_MESSAGE_ERROR: {
            gchar *debug = NULL;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &debug);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            if (debug) {
                g_print("Debug details: %s\n", debug);
                g_free(debug);
            }
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(test->pipeline),
                                              GST_DEBUG_GRAPH_SHOW_ALL, "error");
            g_main_loop_quit(loop);
            break;
        }
        case GST_MESSAGE_INFO : {
            gchar *info = NULL;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &info);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            if (info) {
                g_print("Debug details: %s\n", info);
                g_free(info);
            }
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(test->pipeline),
                                              GST_DEBUG_GRAPH_SHOW_ALL, "info");
            break;
        }
        case GST_MESSAGE_WARNING : {
            gchar *warr = NULL;
            GError *err = NULL;
            gst_message_parse_error(msg, &err, &warr);
            g_print("Error: %s\n", err->message);
            g_error_free(err);
            if (warr) {
                g_print("Debug details: %s\n", warr);
                g_free(warr);
            }
            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(test->pipeline),
                                              GST_DEBUG_GRAPH_SHOW_ALL, "warning");
            break;
        }


        case GST_MESSAGE_STREAM_STATUS: {
            GstStreamStatusType *stats = NULL;
            GstElement *owner;
            const GValue *val;
            gchar *path;
            GST_WARNING ("received STREAM_STATUS");
            gst_message_parse_stream_status(msg, stats, &owner);

            val = gst_message_get_stream_status_object(msg);

            path = gst_object_get_path_string(GST_MESSAGE_SRC (msg));
            GST_WARNING ("source: %s", path);
            g_free(path);
            path = gst_object_get_path_string(GST_OBJECT (owner));
            GST_WARNING ("owner:  %s", path);
            g_free(path);
            GST_WARNING ("object: type %s, value %p", G_VALUE_TYPE_NAME(val), g_value_get_object(val));


            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(GST_BIN(test->pipeline), GST_DEBUG_GRAPH_SHOW_ALL,
                                              G_VALUE_TYPE_NAME(val));
            break;
        }

        case GST_MESSAGE_STATE_CHANGED : {
            GstState old_state, new_state;

            gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
            g_print("Element %s changed state from %s to %s.\n", GST_OBJECT_NAME (msg->src),
                    gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));


            GST_DEBUG_BIN_TO_DOT_FILE_WITH_TS(
                    GST_BIN(test->pipeline),
                    GST_DEBUG_GRAPH_SHOW_ALL,
                    g_strconcat(gst_element_state_get_name(old_state),
                                "_", gst_element_state_get_name(new_state)));
            break;
        }

        default:
            break;
    }

    return TRUE;
}

static void demux_new_pad(GstElement *demux,
                          GstPad *pad,
                          Test *test) {

    const gchar *cap_name, *cap_media;
    gchar *pad_name;
    GstCaps *caps;
    GstPad *sinkpad;
    GstElement *tee, *sink, *queue;
    GstStructure *s;
    gint cap_size;

    pad_name = gst_pad_get_name (pad);
    g_print("A new pad %s was created\n", pad_name);
    g_free(pad_name);

    /* here, you would setup a new pad link for the newly created pad */
    caps = gst_pad_query_caps(pad, NULL);
    cap_size = gst_caps_get_size(caps);

    GST_WARNING("mpegts pad created %s for element %s \n",
                gst_structure_get_name(gst_caps_get_structure(caps, 0)), gst_element_get_name(demux));
    GST_WARNING("CAPS SIZE : %d \n", cap_size);

    s = gst_caps_get_structure(caps, 0);

    cap_name = gst_structure_get_name(s);
    cap_media = gst_structure_get_string(s, "media");

    GST_WARNING("caps name :  %s", cap_name);
    GST_WARNING("caps media :  %s", cap_media);

    if (!strncmp(cap_name, "video", 5)) {
        gchar num = test->queue_id++;
        g_autofree gchar *queue_nickname = g_strconcat("queue_", num, NULL);
        queue = gst_element_factory_make("queue", "queue_");
        tee = gst_element_factory_make("tee", "videotee");
        sink = gst_bin_get_by_name(GST_BIN(test->pipeline), "avi_file_sink");

        gst_bin_add(GST_BIN (test->pipeline), tee);
        gst_element_link_many(tee, queue, sink, NULL);
        sinkpad = gst_element_get_static_pad(tee, "sink");
        gst_pad_link(pad, sinkpad);
        gst_object_unref(sinkpad);
        gst_element_set_state(tee, GST_STATE_PLAYING);
        gst_element_set_state(sink, GST_STATE_PLAYING);
        GST_WARNING ("Linked pad %s of demuxer\n", pad_name);
    }
    if (!strncmp(cap_name, "private", 7)) {
        queue = gst_bin_get_by_name(GST_BIN(test->pipeline), "queue4");
        tee = gst_element_factory_make("tee", "private_tee");
        sink = gst_bin_get_by_name(GST_BIN(test->pipeline), "unknown_sink");

        gst_bin_add(GST_BIN (test->pipeline), tee);
        gst_element_link_many(tee, queue, sink, NULL);
        sinkpad = gst_element_get_static_pad(tee, "sink");
        gst_pad_link(pad, sinkpad);
        gst_object_unref(sinkpad);
        gst_element_set_state(tee, GST_STATE_PLAYING);
        gst_element_set_state(sink, GST_STATE_PLAYING);
        GST_WARNING ("Linked pad %s of demuxer\n", pad_name);
    }


}


void gst_link_and_test_elements(GstElement *ele1, GstElement *ele2) {
    gchar *name_ele1, *name_ele2;

    name_ele1 = gst_element_get_name(ele1);
    name_ele2 = gst_element_get_name(ele2);

    if (!ele1) {
        GST_ERROR("element %s badly created", name_ele1);
        exit(EXIT_FAILURE);
    }

    if (!ele2) {
        GST_ERROR("element %s badly created", name_ele2);
        exit(EXIT_FAILURE);
    }

    if (!gst_element_link(ele1, ele2)) {
        GST_ERROR("link between %s and %s didn't work", name_ele1, name_ele2);
        exit(EXIT_FAILURE);
    }

}

void gst_link_and_test_elements_with_caps(GstElement *ele1, GstElement *ele2, GstCaps *caps) {
    gchar *name_ele1, *name_ele2;

    name_ele1 = gst_element_get_name(ele1);
    name_ele2 = gst_element_get_name(ele2);

    if (!ele1) {
        GST_ERROR("element %s badly created", name_ele1);
        exit(EXIT_FAILURE);
    }

    if (!ele2) {
        GST_ERROR("element %s badly created", name_ele2);
        exit(EXIT_FAILURE);
    }

    if (!caps) {
        GST_ERROR("caps badly created");
        exit(EXIT_FAILURE);
    }


    if (!gst_element_link_filtered(ele1, ele2, caps)) {
        GST_ERROR("link between %s and %s didn't work", name_ele1, name_ele2);
        exit(EXIT_FAILURE);
    }

}

void test_mux( const gchar *ts_location) {
    Test *test = &s_test;

    GstElement *fakesrc;
    GstElement *identity;
    GstElement *mux;
    GstElement *videotestsrc;
    GstElement *x264encoder;
    GstElement *mts_sink;

    // partie mux
    videotestsrc = gst_element_factory_make("videotestsrc", "videotest");
    g_object_set(G_OBJECT(videotestsrc), "num-buffers", 50, NULL);
    mux = gst_element_factory_make("mpegtsmux", "my_ts_muxer");
    mts_sink = gst_element_factory_make("filesink", "mts_sink");
    g_object_set(G_OBJECT(mts_sink), "location", ts_location, NULL);
    x264encoder = gst_element_factory_make("x264enc", "264encoder");
    fakesrc = gst_element_factory_make("fakesrc", "faksrc");
    g_object_set(G_OBJECT (fakesrc), "do-timestamp", TRUE, NULL);
    g_object_set(G_OBJECT (fakesrc), "blocksize", 80, NULL);
    g_object_set(G_OBJECT (fakesrc), "num-buffers", 50, NULL);
    g_object_set(G_OBJECT (fakesrc), "sizetype", 1, NULL);
    g_object_set(G_OBJECT (fakesrc), "format", 3, NULL);
    g_object_set(G_OBJECT (fakesrc), "filltype", 3, NULL);
    identity = gst_element_factory_make("identity", "identity");
    g_object_set(G_OBJECT(identity), "sync", TRUE, NULL);



    gst_bin_add_many(GST_BIN(test->pipeline), videotestsrc, mux, mts_sink,
                     x264encoder, fakesrc, identity, NULL);

    GST_WARNING("LINKING MUXING PART");

    gst_link_and_test_elements(fakesrc, identity);

    gst_link_and_test_elements(identity, mux);

    gst_link_and_test_elements(videotestsrc, x264encoder);

    gst_link_and_test_elements(x264encoder, mux);

    gst_link_and_test_elements(mux, mts_sink);

    GST_WARNING("END LINKING MUXING PART");

}


void test_demux(const gchar *ts_location) {

    Test *test = &s_test;
    GstElement *filesrc;
    GstElement *typefind3;
    GstElement *demux;

    // partie demux
    filesrc = gst_element_factory_make("filesrc", "my_filesource");
    g_object_set(G_OBJECT(filesrc), "location", ts_location, NULL);
    demux = gst_element_factory_make("tsdemux", "my_ts_demuxer");
    typefind3 = gst_element_factory_make("typefind", "typefind3");



    // add all element into the pipeline
    gst_bin_add_many(GST_BIN(test->pipeline), filesrc, demux, typefind3, NULL);

    GST_WARNING("DEMUXING PAD-ADD SIGNAL");
    g_signal_connect(demux, "pad-added", G_CALLBACK(demux_new_pad), NULL);

    // LINKINK THE DEMUXING PART
    GST_WARNING("LINKING DEMUXING PART");

    gst_link_and_test_elements(filesrc, typefind3);

    gst_link_and_test_elements(typefind3, demux);

    GST_WARNING("LINKING DEMUXING PART");

}


gint main(gint argc, gchar *argv[]) {

    Test *test = &s_test;

    GstStateChangeReturn ret;

    /* initialization */
    gst_init(&argc, &argv);
    test->loop = g_main_loop_new(NULL, TRUE);

    if (argc != 3) {
        g_print("Usage: %s <test file folder location> <mode>\n", argv[0]);
        g_print("mode: \n");
        g_print("\t0:all \n");
        g_print("\t1:mux \n");
        g_print("\t2:demux \n");
        return -1;
    }

    const gchar *folder = argv[1];
    int test_mode = atoi(argv[2]);

    const char *mts_file = "/lol.mts";
    g_autofree gchar *ts_location = g_strconcat(folder, mts_file, NULL);

    const char *avi_file = "/lol.avi";
    g_autofree gchar *avi_location = g_strconcat(folder, avi_file, NULL);

    const char *app_file = "/lol";
    g_autofree gchar *lol_location = g_strconcat(folder, app_file, NULL);

    g_print(" test mode %d ts location : %s \n", test_mode, ts_location);


    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    test->pipeline_name = "my_pipeline";
    test->pipeline = gst_pipeline_new(test->pipeline_name);

    test->bus = gst_pipeline_get_bus(GST_PIPELINE (test->pipeline));
    test->watch_id = gst_bus_add_watch(test->bus, (GstBusFunc) bus_call, test);
    gst_object_unref(test->bus);


    switch (test_mode) {
        case 0 : {
            // LINKING THE MUX PART
            GST_WARNING("FULL TEST...");
            test_mux(ts_location);

            test_demux(ts_location);

            break;
        }
        case 1 : {
            GST_WARNING("MUX TEST...");
            test_mux(ts_location);

            break;
        }
        case 2 : {
            GST_WARNING("DEMUX TEST...");
            test_demux(ts_location);

            break;
        }
        default:
            GST_ERROR("test mode unknown : %d", test_mode);
            break;

    }



    ret = gst_element_set_state(test->pipeline, GST_STATE_PLAYING);

    if (ret == GST_STATE_CHANGE_FAILURE) {
        GstMessage *msg;

        g_print("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        msg = gst_bus_poll(test->bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error(msg, &err, NULL);
            g_print("ERROR: %s\n", err->message);
            g_error_free(err);
            gst_message_unref(msg);
        }
        return -1;
    }


    g_print("Running...\n");
    g_main_loop_run(test->loop);

    /* clean up */
    g_print("Returned, stopping playback\n");
    gst_element_set_state(test->pipeline, GST_STATE_NULL);

    g_print("Deleting pipeline\n");
    gst_object_unref(test->pipeline);
    g_source_remove(test->watch_id);
    g_main_loop_unref(test->loop);

    return 0;
}

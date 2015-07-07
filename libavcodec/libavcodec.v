LIBAVCODEC_$MAJOR {
        global: av*;
                ff*;
                #deprecated, remove after next bump
                audio_resample;
                audio_resample_close;
        local:  *;
};

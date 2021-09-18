LIBAVCODEC_MAJOR {
    global:
<<<<<<< HEAD
        av*;
                ff*;
                speex*;
        #deprecated, remove after next bump
        audio_resample;
        audio_resample_close;
=======
        av_*;
        avcodec_*;
        avpriv_*;
        avsubtitle_free;
>>>>>>> 4fc0b75973d20425df22a9178fc2b735710a5f40
    local:
        *;
};


<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN"
          "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
  
<meta http-equiv="X-UA-Compatible" content="chrome=1">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<title>libavfilter/af_sox.c - sastes-ffmpeg in FFmpeg - Gitorious</title>
<link href="/stylesheets/gts-common.css?1335784269" media="screen" rel="stylesheet" type="text/css" />
<link href="//fonts.googleapis.com/css?family=Nobile&amp;v1" type="text/css" rel="stylesheet">

<script src="/javascripts/all.js?1335784291" type="text/javascript"></script>      <link href="/stylesheets/prettify/prettify.css?1335784269" media="screen" rel="stylesheet" type="text/css" />    <script src="/javascripts/lib/prettify.js?1335784269" type="text/javascript"></script>      <!--[if IE 8]><link rel="stylesheet" href="/stylesheets/ie8.css" type="text/css"><![endif]-->
<!--[if IE 7]><link rel="stylesheet" href="/stylesheets/ie7.css" type="text/css"><![endif]-->
<script type="text/javascript">
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-52238-3']);
_gaq.push(['_setDomainName', '.gitorious.org'])
_gaq.push(['_trackPageview']);
(function() {
   var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
   ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
   (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(ga);
})();
</script>
</head>
<body id="blobs">
  <div id="wrapper">
        <ul id="user-nav">
      <li><a href="/">Dashboard</a></li>
      
          <li class="secondary"><a href="/users/new">Register</a></li>
    <li class="secondary"><a href="/login">Login</a></li>
    </ul>
    <div id="header">
      <h1 id="logo">
        <a href="/"><img alt="Logo" src="/img/logo.png?1294322727" /></a>
      </h1>
      <ul id="menu">
                  <li class="activity"><a href="/activities">Activities</a></li>
          <li class="projects"><a href="/projects">Projects</a></li>
          <li class="teams"><a href="/teams">Teams</a></li>
              </ul>
    </div>
    <div id="top-bar">
      <ul id="breadcrumbs">
        <li class="user"><a href="/~saste">Stefano Sabatini</a></li><li class="repository"><a href="/~saste/ffmpeg/sastes-ffmpeg">sastes-ffmpeg</a></li><li class="branch"><a href="/~saste/ffmpeg/sastes-ffmpeg/commits/audio-filters-20121203">audio-filters-20121203</a></li><li class="tree"><a href="/~saste/ffmpeg/sastes-ffmpeg/trees/audio-filters-20121203">/</a></li><li class="folder"><a href="/~saste/ffmpeg/sastes-ffmpeg/trees/audio-filters-20121203/libavfilter">libavfilter</a></li><li class="file"><a href="/~saste/ffmpeg/sastes-ffmpeg/blobs/audio-filters-20121203/libavfilter/af_sox.c">af_sox.c</a></li>      </ul>
              <div id="searchbox">
          


<div class="search_bar">
  <form action="/search" method="get">    <p>
      <input class="text search-field round-5" id="q" name="q" type="text" />      <input type="submit" value="Search" class="search-submit round-5" />
    </p>
    <p class="hint search-hint" style="display: none;">
      eg. 'wrapper', 'category:python' or '"document database"'
          </p>
  </form></div>
        </div>
          </div>
    <div id="container" class="">
      <div id="content" class="">
        
        



<div class="page-meta">
  <ul class="page-actions">
    <li>Blob contents</li>
    <li><a href="/~saste/ffmpeg/sastes-ffmpeg/blobs/blame/e6a3fd54aff3b22759ee77ffc67bd39cd63f5810/libavfilter/af_sox.c" class="blame js-pjax" data-pjax="#codeblob">Blame</a></li>
    <li><a href="/~saste/ffmpeg/sastes-ffmpeg/blobs/history/audio-filters-20121203/libavfilter/af_sox.c" class="js-pjax" data-pjax="#codeblob">Blob history</a></li>
    <li><a href="/~saste/ffmpeg/sastes-ffmpeg/blobs/raw/audio-filters-20121203/libavfilter/af_sox.c">Raw blob data</a></li>
  </ul>
</div>



<!-- mime: text/plain -->

       <div id="long-file" style="display:none"
                  class="help-box center error round-5">
               <div class="icon error"></div>        <p>
          This file looks large and may slow your browser down if we attempt
          to syntax highlight it, so we are showing it without any
          pretty colors.
          <a href="#highlight-anyway" id="highlight-anyway">Highlight
          it anyway</a>.
        </p>
     </div>    <table id="codeblob" class="highlighted lang-c">
<tr id="line1">
<td class="line-numbers"><a href="#line1" name="line1">1</a></td>
<td class="code"><pre class="prettyprint lang-c">/*</pre></td>
</tr>
<tr id="line2">
<td class="line-numbers"><a href="#line2" name="line2">2</a></td>
<td class="code"><pre class="prettyprint lang-c"> * Copyright (c) 2011 Mina Nagy Zaki</pre></td>
</tr>
<tr id="line3">
<td class="line-numbers"><a href="#line3" name="line3">3</a></td>
<td class="code"><pre class="prettyprint lang-c"> *</pre></td>
</tr>
<tr id="line4">
<td class="line-numbers"><a href="#line4" name="line4">4</a></td>
<td class="code"><pre class="prettyprint lang-c"> * This file is part of FFmpeg.</pre></td>
</tr>
<tr id="line5">
<td class="line-numbers"><a href="#line5" name="line5">5</a></td>
<td class="code"><pre class="prettyprint lang-c"> *</pre></td>
</tr>
<tr id="line6">
<td class="line-numbers"><a href="#line6" name="line6">6</a></td>
<td class="code"><pre class="prettyprint lang-c"> * FFmpeg is free software; you can redistribute it and/or</pre></td>
</tr>
<tr id="line7">
<td class="line-numbers"><a href="#line7" name="line7">7</a></td>
<td class="code"><pre class="prettyprint lang-c"> * modify it under the terms of the GNU Lesser General Public</pre></td>
</tr>
<tr id="line8">
<td class="line-numbers"><a href="#line8" name="line8">8</a></td>
<td class="code"><pre class="prettyprint lang-c"> * License as published by the Free Software Foundation; either</pre></td>
</tr>
<tr id="line9">
<td class="line-numbers"><a href="#line9" name="line9">9</a></td>
<td class="code"><pre class="prettyprint lang-c"> * version 2.1 of the License, or (at your option) any later version.</pre></td>
</tr>
<tr id="line10">
<td class="line-numbers"><a href="#line10" name="line10">10</a></td>
<td class="code"><pre class="prettyprint lang-c"> *</pre></td>
</tr>
<tr id="line11">
<td class="line-numbers"><a href="#line11" name="line11">11</a></td>
<td class="code"><pre class="prettyprint lang-c"> * FFmpeg is distributed in the hope that it will be useful,</pre></td>
</tr>
<tr id="line12">
<td class="line-numbers"><a href="#line12" name="line12">12</a></td>
<td class="code"><pre class="prettyprint lang-c"> * but WITHOUT ANY WARRANTY; without even the implied warranty of</pre></td>
</tr>
<tr id="line13">
<td class="line-numbers"><a href="#line13" name="line13">13</a></td>
<td class="code"><pre class="prettyprint lang-c"> * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU</pre></td>
</tr>
<tr id="line14">
<td class="line-numbers"><a href="#line14" name="line14">14</a></td>
<td class="code"><pre class="prettyprint lang-c"> * Lesser General Public License for more details.</pre></td>
</tr>
<tr id="line15">
<td class="line-numbers"><a href="#line15" name="line15">15</a></td>
<td class="code"><pre class="prettyprint lang-c"> *</pre></td>
</tr>
<tr id="line16">
<td class="line-numbers"><a href="#line16" name="line16">16</a></td>
<td class="code"><pre class="prettyprint lang-c"> * You should have received a copy of the GNU Lesser General Public</pre></td>
</tr>
<tr id="line17">
<td class="line-numbers"><a href="#line17" name="line17">17</a></td>
<td class="code"><pre class="prettyprint lang-c"> * License along with FFmpeg; if not, write to the Free Software</pre></td>
</tr>
<tr id="line18">
<td class="line-numbers"><a href="#line18" name="line18">18</a></td>
<td class="code"><pre class="prettyprint lang-c"> * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA</pre></td>
</tr>
<tr id="line19">
<td class="line-numbers"><a href="#line19" name="line19">19</a></td>
<td class="code"><pre class="prettyprint lang-c"> */</pre></td>
</tr>
<tr id="line20">
<td class="line-numbers"><a href="#line20" name="line20">20</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line21">
<td class="line-numbers"><a href="#line21" name="line21">21</a></td>
<td class="code"><pre class="prettyprint lang-c">/**</pre></td>
</tr>
<tr id="line22">
<td class="line-numbers"><a href="#line22" name="line22">22</a></td>
<td class="code"><pre class="prettyprint lang-c"> * @file</pre></td>
</tr>
<tr id="line23">
<td class="line-numbers"><a href="#line23" name="line23">23</a></td>
<td class="code"><pre class="prettyprint lang-c"> * libsox wrapper filter</pre></td>
</tr>
<tr id="line24">
<td class="line-numbers"><a href="#line24" name="line24">24</a></td>
<td class="code"><pre class="prettyprint lang-c"> */</pre></td>
</tr>
<tr id="line25">
<td class="line-numbers"><a href="#line25" name="line25">25</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line26">
<td class="line-numbers"><a href="#line26" name="line26">26</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &lt;sox.h&gt;</pre></td>
</tr>
<tr id="line27">
<td class="line-numbers"><a href="#line27" name="line27">27</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &lt;string.h&gt;</pre></td>
</tr>
<tr id="line28">
<td class="line-numbers"><a href="#line28" name="line28">28</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line29">
<td class="line-numbers"><a href="#line29" name="line29">29</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &quot;libavutil/avstring.h&quot;</pre></td>
</tr>
<tr id="line30">
<td class="line-numbers"><a href="#line30" name="line30">30</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &quot;libavutil/audioconvert.h&quot;</pre></td>
</tr>
<tr id="line31">
<td class="line-numbers"><a href="#line31" name="line31">31</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &quot;libavutil/mem.h&quot;</pre></td>
</tr>
<tr id="line32">
<td class="line-numbers"><a href="#line32" name="line32">32</a></td>
<td class="code"><pre class="prettyprint lang-c">#include &quot;avfilter.h&quot;</pre></td>
</tr>
<tr id="line33">
<td class="line-numbers"><a href="#line33" name="line33">33</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line34">
<td class="line-numbers"><a href="#line34" name="line34">34</a></td>
<td class="code"><pre class="prettyprint lang-c">static int soxinit = -1;</pre></td>
</tr>
<tr id="line35">
<td class="line-numbers"><a href="#line35" name="line35">35</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line36">
<td class="line-numbers"><a href="#line36" name="line36">36</a></td>
<td class="code"><pre class="prettyprint lang-c">#define NUM_ADDED_ARGS 5</pre></td>
</tr>
<tr id="line37">
<td class="line-numbers"><a href="#line37" name="line37">37</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line38">
<td class="line-numbers"><a href="#line38" name="line38">38</a></td>
<td class="code"><pre class="prettyprint lang-c">typedef struct {</pre></td>
</tr>
<tr id="line39">
<td class="line-numbers"><a href="#line39" name="line39">39</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_effect_t *effect;</pre></td>
</tr>
<tr id="line40">
<td class="line-numbers"><a href="#line40" name="line40">40</a></td>
<td class="code"><pre class="prettyprint lang-c">} SoxContext;</pre></td>
</tr>
<tr id="line41">
<td class="line-numbers"><a href="#line41" name="line41">41</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line42">
<td class="line-numbers"><a href="#line42" name="line42">42</a></td>
<td class="code"><pre class="prettyprint lang-c">/* sox needs a variable number of params */</pre></td>
</tr>
<tr id="line43">
<td class="line-numbers"><a href="#line43" name="line43">43</a></td>
<td class="code"><pre class="prettyprint lang-c">static inline void realloc_argv(char ***argv, int *numargs)</pre></td>
</tr>
<tr id="line44">
<td class="line-numbers"><a href="#line44" name="line44">44</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line45">
<td class="line-numbers"><a href="#line45" name="line45">45</a></td>
<td class="code"><pre class="prettyprint lang-c">    *numargs += NUM_ADDED_ARGS;</pre></td>
</tr>
<tr id="line46">
<td class="line-numbers"><a href="#line46" name="line46">46</a></td>
<td class="code"><pre class="prettyprint lang-c">    *argv = av_realloc(*argv, *numargs * sizeof(**argv));</pre></td>
</tr>
<tr id="line47">
<td class="line-numbers"><a href="#line47" name="line47">47</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (!*argv)</pre></td>
</tr>
<tr id="line48">
<td class="line-numbers"><a href="#line48" name="line48">48</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_free(*argv);</pre></td>
</tr>
<tr id="line49">
<td class="line-numbers"><a href="#line49" name="line49">49</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line50">
<td class="line-numbers"><a href="#line50" name="line50">50</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line51">
<td class="line-numbers"><a href="#line51" name="line51">51</a></td>
<td class="code"><pre class="prettyprint lang-c">static av_cold int init(AVFilterContext *ctx, const char *args0, void *opaque)</pre></td>
</tr>
<tr id="line52">
<td class="line-numbers"><a href="#line52" name="line52">52</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line53">
<td class="line-numbers"><a href="#line53" name="line53">53</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = ctx-&gt;priv;</pre></td>
</tr>
<tr id="line54">
<td class="line-numbers"><a href="#line54" name="line54">54</a></td>
<td class="code"><pre class="prettyprint lang-c">    char **argv = NULL; int argc = 0, numargs = 0;</pre></td>
</tr>
<tr id="line55">
<td class="line-numbers"><a href="#line55" name="line55">55</a></td>
<td class="code"><pre class="prettyprint lang-c">    char *args, *ptr = NULL;</pre></td>
</tr>
<tr id="line56">
<td class="line-numbers"><a href="#line56" name="line56">56</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_encodinginfo_t *enc;</pre></td>
</tr>
<tr id="line57">
<td class="line-numbers"><a href="#line57" name="line57">57</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line58">
<td class="line-numbers"><a href="#line58" name="line58">58</a></td>
<td class="code"><pre class="prettyprint lang-c">    // initialize SoX if necessary</pre></td>
</tr>
<tr id="line59">
<td class="line-numbers"><a href="#line59" name="line59">59</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (soxinit != SOX_SUCCESS &amp;&amp; (soxinit = sox_init()) != SOX_SUCCESS) {</pre></td>
</tr>
<tr id="line60">
<td class="line-numbers"><a href="#line60" name="line60">60</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_log(ctx, AV_LOG_ERROR, &quot;Sox error occurred: '%s'\n&quot;,</pre></td>
</tr>
<tr id="line61">
<td class="line-numbers"><a href="#line61" name="line61">61</a></td>
<td class="code"><pre class="prettyprint lang-c">               sox_strerror(soxinit));</pre></td>
</tr>
<tr id="line62">
<td class="line-numbers"><a href="#line62" name="line62">62</a></td>
<td class="code"><pre class="prettyprint lang-c">        return AVERROR(EINVAL);</pre></td>
</tr>
<tr id="line63">
<td class="line-numbers"><a href="#line63" name="line63">63</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line64">
<td class="line-numbers"><a href="#line64" name="line64">64</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line65">
<td class="line-numbers"><a href="#line65" name="line65">65</a></td>
<td class="code"><pre class="prettyprint lang-c">    args = av_strdup(args0);</pre></td>
</tr>
<tr id="line66">
<td class="line-numbers"><a href="#line66" name="line66">66</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line67">
<td class="line-numbers"><a href="#line67" name="line67">67</a></td>
<td class="code"><pre class="prettyprint lang-c">    /* create an array of arguments to pass to sox_create_effect */</pre></td>
</tr>
<tr id="line68">
<td class="line-numbers"><a href="#line68" name="line68">68</a></td>
<td class="code"><pre class="prettyprint lang-c">    realloc_argv(&amp;argv, &amp;numargs);</pre></td>
</tr>
<tr id="line69">
<td class="line-numbers"><a href="#line69" name="line69">69</a></td>
<td class="code"><pre class="prettyprint lang-c">    argv[argc++] = av_strtok(args, &quot; &quot;, &amp;ptr);</pre></td>
</tr>
<tr id="line70">
<td class="line-numbers"><a href="#line70" name="line70">70</a></td>
<td class="code"><pre class="prettyprint lang-c">    while (argv[argc++] = av_strtok(NULL, &quot; &quot;, &amp;ptr)) {</pre></td>
</tr>
<tr id="line71">
<td class="line-numbers"><a href="#line71" name="line71">71</a></td>
<td class="code"><pre class="prettyprint lang-c">        if (argc == numargs) {</pre></td>
</tr>
<tr id="line72">
<td class="line-numbers"><a href="#line72" name="line72">72</a></td>
<td class="code"><pre class="prettyprint lang-c">            realloc_argv(&amp;argv, &amp;numargs);</pre></td>
</tr>
<tr id="line73">
<td class="line-numbers"><a href="#line73" name="line73">73</a></td>
<td class="code"><pre class="prettyprint lang-c">            if (!numargs) {</pre></td>
</tr>
<tr id="line74">
<td class="line-numbers"><a href="#line74" name="line74">74</a></td>
<td class="code"><pre class="prettyprint lang-c">                av_log(ctx, AV_LOG_ERROR, &quot;Could not allocate memory!\n&quot;);</pre></td>
</tr>
<tr id="line75">
<td class="line-numbers"><a href="#line75" name="line75">75</a></td>
<td class="code"><pre class="prettyprint lang-c">                av_free(args);</pre></td>
</tr>
<tr id="line76">
<td class="line-numbers"><a href="#line76" name="line76">76</a></td>
<td class="code"><pre class="prettyprint lang-c">                return AVERROR(ENOMEM);</pre></td>
</tr>
<tr id="line77">
<td class="line-numbers"><a href="#line77" name="line77">77</a></td>
<td class="code"><pre class="prettyprint lang-c">            }</pre></td>
</tr>
<tr id="line78">
<td class="line-numbers"><a href="#line78" name="line78">78</a></td>
<td class="code"><pre class="prettyprint lang-c">        }</pre></td>
</tr>
<tr id="line79">
<td class="line-numbers"><a href="#line79" name="line79">79</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line80">
<td class="line-numbers"><a href="#line80" name="line80">80</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line81">
<td class="line-numbers"><a href="#line81" name="line81">81</a></td>
<td class="code"><pre class="prettyprint lang-c">    // Create the effect</pre></td>
</tr>
<tr id="line82">
<td class="line-numbers"><a href="#line82" name="line82">82</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox-&gt;effect = sox_create_effect(sox_find_effect(argv[0]));</pre></td>
</tr>
<tr id="line83">
<td class="line-numbers"><a href="#line83" name="line83">83</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (!sox-&gt;effect) {</pre></td>
</tr>
<tr id="line84">
<td class="line-numbers"><a href="#line84" name="line84">84</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_log(ctx, AV_LOG_ERROR, &quot;Could not create Sox effect '%s'\n&quot;, argv[0]);</pre></td>
</tr>
<tr id="line85">
<td class="line-numbers"><a href="#line85" name="line85">85</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_free(args);</pre></td>
</tr>
<tr id="line86">
<td class="line-numbers"><a href="#line86" name="line86">86</a></td>
<td class="code"><pre class="prettyprint lang-c">        return AVERROR(EINVAL);</pre></td>
</tr>
<tr id="line87">
<td class="line-numbers"><a href="#line87" name="line87">87</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line88">
<td class="line-numbers"><a href="#line88" name="line88">88</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line89">
<td class="line-numbers"><a href="#line89" name="line89">89</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (sox-&gt;effect-&gt;handler.getopts(sox-&gt;effect, argc-1, argv) != SOX_SUCCESS) {</pre></td>
</tr>
<tr id="line90">
<td class="line-numbers"><a href="#line90" name="line90">90</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_log(ctx, AV_LOG_ERROR, &quot;Invalid arguments to Sox effect '%s'\n&quot;, argv[0]);</pre></td>
</tr>
<tr id="line91">
<td class="line-numbers"><a href="#line91" name="line91">91</a></td>
<td class="code"><pre class="prettyprint lang-c">        if (sox-&gt;effect-&gt;handler.usage)</pre></td>
</tr>
<tr id="line92">
<td class="line-numbers"><a href="#line92" name="line92">92</a></td>
<td class="code"><pre class="prettyprint lang-c">            av_log(ctx, AV_LOG_ERROR, &quot;Usage: %s\n&quot;, sox-&gt;effect-&gt;handler.usage);</pre></td>
</tr>
<tr id="line93">
<td class="line-numbers"><a href="#line93" name="line93">93</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_free(args);</pre></td>
</tr>
<tr id="line94">
<td class="line-numbers"><a href="#line94" name="line94">94</a></td>
<td class="code"><pre class="prettyprint lang-c">        return AVERROR(EINVAL);</pre></td>
</tr>
<tr id="line95">
<td class="line-numbers"><a href="#line95" name="line95">95</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line96">
<td class="line-numbers"><a href="#line96" name="line96">96</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line97">
<td class="line-numbers"><a href="#line97" name="line97">97</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (!(enc = av_malloc(sizeof(sox_encodinginfo_t)))) {</pre></td>
</tr>
<tr id="line98">
<td class="line-numbers"><a href="#line98" name="line98">98</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_free(args);</pre></td>
</tr>
<tr id="line99">
<td class="line-numbers"><a href="#line99" name="line99">99</a></td>
<td class="code"><pre class="prettyprint lang-c">        return AVERROR(ENOMEM);</pre></td>
</tr>
<tr id="line100">
<td class="line-numbers"><a href="#line100" name="line100">100</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line101">
<td class="line-numbers"><a href="#line101" name="line101">101</a></td>
<td class="code"><pre class="prettyprint lang-c">    memset(enc, 0, sizeof(sox_encodinginfo_t));</pre></td>
</tr>
<tr id="line102">
<td class="line-numbers"><a href="#line102" name="line102">102</a></td>
<td class="code"><pre class="prettyprint lang-c">    enc-&gt;bits_per_sample = 32;</pre></td>
</tr>
<tr id="line103">
<td class="line-numbers"><a href="#line103" name="line103">103</a></td>
<td class="code"><pre class="prettyprint lang-c">    enc-&gt;encoding        = SOX_DEFAULT_ENCODING;</pre></td>
</tr>
<tr id="line104">
<td class="line-numbers"><a href="#line104" name="line104">104</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox-&gt;effect-&gt;out_encoding = sox-&gt;effect-&gt;in_encoding = enc;</pre></td>
</tr>
<tr id="line105">
<td class="line-numbers"><a href="#line105" name="line105">105</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox-&gt;effect-&gt;clips        = 0;</pre></td>
</tr>
<tr id="line106">
<td class="line-numbers"><a href="#line106" name="line106">106</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox-&gt;effect-&gt;imin         = 0;</pre></td>
</tr>
<tr id="line107">
<td class="line-numbers"><a href="#line107" name="line107">107</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line108">
<td class="line-numbers"><a href="#line108" name="line108">108</a></td>
<td class="code"><pre class="prettyprint lang-c">    av_free(args);</pre></td>
</tr>
<tr id="line109">
<td class="line-numbers"><a href="#line109" name="line109">109</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line110">
<td class="line-numbers"><a href="#line110" name="line110">110</a></td>
<td class="code"><pre class="prettyprint lang-c">    return 0;</pre></td>
</tr>
<tr id="line111">
<td class="line-numbers"><a href="#line111" name="line111">111</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line112">
<td class="line-numbers"><a href="#line112" name="line112">112</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line113">
<td class="line-numbers"><a href="#line113" name="line113">113</a></td>
<td class="code"><pre class="prettyprint lang-c">static av_cold void uninit(AVFilterContext *ctx)</pre></td>
</tr>
<tr id="line114">
<td class="line-numbers"><a href="#line114" name="line114">114</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line115">
<td class="line-numbers"><a href="#line115" name="line115">115</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = ctx-&gt;priv;</pre></td>
</tr>
<tr id="line116">
<td class="line-numbers"><a href="#line116" name="line116">116</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line117">
<td class="line-numbers"><a href="#line117" name="line117">117</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (sox-&gt;effect)</pre></td>
</tr>
<tr id="line118">
<td class="line-numbers"><a href="#line118" name="line118">118</a></td>
<td class="code"><pre class="prettyprint lang-c">        sox_delete_effect(sox-&gt;effect);</pre></td>
</tr>
<tr id="line119">
<td class="line-numbers"><a href="#line119" name="line119">119</a></td>
<td class="code"><pre class="prettyprint lang-c">    ctx-&gt;priv = NULL;</pre></td>
</tr>
<tr id="line120">
<td class="line-numbers"><a href="#line120" name="line120">120</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_quit();</pre></td>
</tr>
<tr id="line121">
<td class="line-numbers"><a href="#line121" name="line121">121</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line122">
<td class="line-numbers"><a href="#line122" name="line122">122</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line123">
<td class="line-numbers"><a href="#line123" name="line123">123</a></td>
<td class="code"><pre class="prettyprint lang-c">static int config_output(AVFilterLink *outlink)</pre></td>
</tr>
<tr id="line124">
<td class="line-numbers"><a href="#line124" name="line124">124</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line125">
<td class="line-numbers"><a href="#line125" name="line125">125</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = outlink-&gt;src-&gt;priv;</pre></td>
</tr>
<tr id="line126">
<td class="line-numbers"><a href="#line126" name="line126">126</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_effect_t *effect = sox-&gt;effect;</pre></td>
</tr>
<tr id="line127">
<td class="line-numbers"><a href="#line127" name="line127">127</a></td>
<td class="code"><pre class="prettyprint lang-c">    AVFilterLink *inlink = outlink-&gt;src-&gt;inputs[0];</pre></td>
</tr>
<tr id="line128">
<td class="line-numbers"><a href="#line128" name="line128">128</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line129">
<td class="line-numbers"><a href="#line129" name="line129">129</a></td>
<td class="code"><pre class="prettyprint lang-c">    effect-&gt;in_signal.precision = 32;</pre></td>
</tr>
<tr id="line130">
<td class="line-numbers"><a href="#line130" name="line130">130</a></td>
<td class="code"><pre class="prettyprint lang-c">    effect-&gt;in_signal.rate      = inlink-&gt;sample_rate;</pre></td>
</tr>
<tr id="line131">
<td class="line-numbers"><a href="#line131" name="line131">131</a></td>
<td class="code"><pre class="prettyprint lang-c">    effect-&gt;in_signal.channels  =</pre></td>
</tr>
<tr id="line132">
<td class="line-numbers"><a href="#line132" name="line132">132</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_get_channel_layout_nb_channels(inlink-&gt;channel_layout);</pre></td>
</tr>
<tr id="line133">
<td class="line-numbers"><a href="#line133" name="line133">133</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line134">
<td class="line-numbers"><a href="#line134" name="line134">134</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (!(effect-&gt;handler.flags &amp; SOX_EFF_CHAN))</pre></td>
</tr>
<tr id="line135">
<td class="line-numbers"><a href="#line135" name="line135">135</a></td>
<td class="code"><pre class="prettyprint lang-c">        effect-&gt;out_signal.channels = effect-&gt;in_signal.channels;</pre></td>
</tr>
<tr id="line136">
<td class="line-numbers"><a href="#line136" name="line136">136</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (!(effect-&gt;handler.flags &amp; SOX_EFF_RATE))</pre></td>
</tr>
<tr id="line137">
<td class="line-numbers"><a href="#line137" name="line137">137</a></td>
<td class="code"><pre class="prettyprint lang-c">        effect-&gt;out_signal.rate = effect-&gt;in_signal.rate;</pre></td>
</tr>
<tr id="line138">
<td class="line-numbers"><a href="#line138" name="line138">138</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line139">
<td class="line-numbers"><a href="#line139" name="line139">139</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (effect-&gt;handler.start(effect) != SOX_SUCCESS) {</pre></td>
</tr>
<tr id="line140">
<td class="line-numbers"><a href="#line140" name="line140">140</a></td>
<td class="code"><pre class="prettyprint lang-c">        av_log(outlink-&gt;src, AV_LOG_ERROR,</pre></td>
</tr>
<tr id="line141">
<td class="line-numbers"><a href="#line141" name="line141">141</a></td>
<td class="code"><pre class="prettyprint lang-c">               &quot;Could not start the sox effect.\n&quot;);</pre></td>
</tr>
<tr id="line142">
<td class="line-numbers"><a href="#line142" name="line142">142</a></td>
<td class="code"><pre class="prettyprint lang-c">        return AVERROR(EINVAL);</pre></td>
</tr>
<tr id="line143">
<td class="line-numbers"><a href="#line143" name="line143">143</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line144">
<td class="line-numbers"><a href="#line144" name="line144">144</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line145">
<td class="line-numbers"><a href="#line145" name="line145">145</a></td>
<td class="code"><pre class="prettyprint lang-c">    outlink-&gt;sample_rate = effect-&gt;out_signal.rate;</pre></td>
</tr>
<tr id="line146">
<td class="line-numbers"><a href="#line146" name="line146">146</a></td>
<td class="code"><pre class="prettyprint lang-c">    outlink-&gt;time_base = (AVRational) {1, effect-&gt;out_signal.rate };</pre></td>
</tr>
<tr id="line147">
<td class="line-numbers"><a href="#line147" name="line147">147</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line148">
<td class="line-numbers"><a href="#line148" name="line148">148</a></td>
<td class="code"><pre class="prettyprint lang-c">    return 0;</pre></td>
</tr>
<tr id="line149">
<td class="line-numbers"><a href="#line149" name="line149">149</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line150">
<td class="line-numbers"><a href="#line150" name="line150">150</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line151">
<td class="line-numbers"><a href="#line151" name="line151">151</a></td>
<td class="code"><pre class="prettyprint lang-c">static int query_formats(AVFilterContext *ctx)</pre></td>
</tr>
<tr id="line152">
<td class="line-numbers"><a href="#line152" name="line152">152</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line153">
<td class="line-numbers"><a href="#line153" name="line153">153</a></td>
<td class="code"><pre class="prettyprint lang-c">    AVFilterLink *outlink = ctx-&gt;outputs[0];</pre></td>
</tr>
<tr id="line154">
<td class="line-numbers"><a href="#line154" name="line154">154</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = ctx-&gt;priv;</pre></td>
</tr>
<tr id="line155">
<td class="line-numbers"><a href="#line155" name="line155">155</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_effect_t *effect = sox-&gt;effect;</pre></td>
</tr>
<tr id="line156">
<td class="line-numbers"><a href="#line156" name="line156">156</a></td>
<td class="code"><pre class="prettyprint lang-c">    AVFilterFormats *formats = NULL;</pre></td>
</tr>
<tr id="line157">
<td class="line-numbers"><a href="#line157" name="line157">157</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line158">
<td class="line-numbers"><a href="#line158" name="line158">158</a></td>
<td class="code"><pre class="prettyprint lang-c">    avfilter_add_format(&amp;formats, AV_SAMPLE_FMT_S32);</pre></td>
</tr>
<tr id="line159">
<td class="line-numbers"><a href="#line159" name="line159">159</a></td>
<td class="code"><pre class="prettyprint lang-c">    avfilter_set_common_sample_formats(ctx, formats);</pre></td>
</tr>
<tr id="line160">
<td class="line-numbers"><a href="#line160" name="line160">160</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line161">
<td class="line-numbers"><a href="#line161" name="line161">161</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (effect-&gt;handler.flags &amp; SOX_EFF_CHAN) {</pre></td>
</tr>
<tr id="line162">
<td class="line-numbers"><a href="#line162" name="line162">162</a></td>
<td class="code"><pre class="prettyprint lang-c">        int64_t chlayout = av_get_default_channel_layout(effect-&gt;out_signal.channels);</pre></td>
</tr>
<tr id="line163">
<td class="line-numbers"><a href="#line163" name="line163">163</a></td>
<td class="code"><pre class="prettyprint lang-c">        if (!chlayout) {</pre></td>
</tr>
<tr id="line164">
<td class="line-numbers"><a href="#line164" name="line164">164</a></td>
<td class="code"><pre class="prettyprint lang-c">            av_log(ctx, AV_LOG_ERROR, &quot;Invalid number of channels '%d' provided\n&quot;,</pre></td>
</tr>
<tr id="line165">
<td class="line-numbers"><a href="#line165" name="line165">165</a></td>
<td class="code"><pre class="prettyprint lang-c">                   effect-&gt;out_signal.channels);</pre></td>
</tr>
<tr id="line166">
<td class="line-numbers"><a href="#line166" name="line166">166</a></td>
<td class="code"><pre class="prettyprint lang-c">            return AVERROR(EINVAL);</pre></td>
</tr>
<tr id="line167">
<td class="line-numbers"><a href="#line167" name="line167">167</a></td>
<td class="code"><pre class="prettyprint lang-c">        }</pre></td>
</tr>
<tr id="line168">
<td class="line-numbers"><a href="#line168" name="line168">168</a></td>
<td class="code"><pre class="prettyprint lang-c">        formats = NULL;</pre></td>
</tr>
<tr id="line169">
<td class="line-numbers"><a href="#line169" name="line169">169</a></td>
<td class="code"><pre class="prettyprint lang-c">        avfilter_add_format(&amp;formats, chlayout);</pre></td>
</tr>
<tr id="line170">
<td class="line-numbers"><a href="#line170" name="line170">170</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line171">
<td class="line-numbers"><a href="#line171" name="line171">171</a></td>
<td class="code"><pre class="prettyprint lang-c">        avfilter_formats_ref(formats,</pre></td>
</tr>
<tr id="line172">
<td class="line-numbers"><a href="#line172" name="line172">172</a></td>
<td class="code"><pre class="prettyprint lang-c">                             &amp;outlink-&gt;in_chlayouts);</pre></td>
</tr>
<tr id="line173">
<td class="line-numbers"><a href="#line173" name="line173">173</a></td>
<td class="code"><pre class="prettyprint lang-c">        avfilter_formats_ref(avfilter_make_all_channel_layouts(),</pre></td>
</tr>
<tr id="line174">
<td class="line-numbers"><a href="#line174" name="line174">174</a></td>
<td class="code"><pre class="prettyprint lang-c">                             &amp;outlink-&gt;out_chlayouts);</pre></td>
</tr>
<tr id="line175">
<td class="line-numbers"><a href="#line175" name="line175">175</a></td>
<td class="code"><pre class="prettyprint lang-c">    } else {</pre></td>
</tr>
<tr id="line176">
<td class="line-numbers"><a href="#line176" name="line176">176</a></td>
<td class="code"><pre class="prettyprint lang-c">        avfilter_set_common_channel_layouts(ctx, avfilter_make_all_channel_layouts());</pre></td>
</tr>
<tr id="line177">
<td class="line-numbers"><a href="#line177" name="line177">177</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line178">
<td class="line-numbers"><a href="#line178" name="line178">178</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line179">
<td class="line-numbers"><a href="#line179" name="line179">179</a></td>
<td class="code"><pre class="prettyprint lang-c">    avfilter_set_common_packing_formats(ctx, avfilter_make_all_packing_formats());</pre></td>
</tr>
<tr id="line180">
<td class="line-numbers"><a href="#line180" name="line180">180</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line181">
<td class="line-numbers"><a href="#line181" name="line181">181</a></td>
<td class="code"><pre class="prettyprint lang-c">    return 0;</pre></td>
</tr>
<tr id="line182">
<td class="line-numbers"><a href="#line182" name="line182">182</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line183">
<td class="line-numbers"><a href="#line183" name="line183">183</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line184">
<td class="line-numbers"><a href="#line184" name="line184">184</a></td>
<td class="code"><pre class="prettyprint lang-c">static int request_frame(AVFilterLink *outlink)</pre></td>
</tr>
<tr id="line185">
<td class="line-numbers"><a href="#line185" name="line185">185</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line186">
<td class="line-numbers"><a href="#line186" name="line186">186</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = outlink-&gt;dst-&gt;priv;</pre></td>
</tr>
<tr id="line187">
<td class="line-numbers"><a href="#line187" name="line187">187</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_effect_t *effect = sox-&gt;effect;</pre></td>
</tr>
<tr id="line188">
<td class="line-numbers"><a href="#line188" name="line188">188</a></td>
<td class="code"><pre class="prettyprint lang-c">    size_t out_nb_samples = 1024;</pre></td>
</tr>
<tr id="line189">
<td class="line-numbers"><a href="#line189" name="line189">189</a></td>
<td class="code"><pre class="prettyprint lang-c">    AVFilterBufferRef *outsamples;</pre></td>
</tr>
<tr id="line190">
<td class="line-numbers"><a href="#line190" name="line190">190</a></td>
<td class="code"><pre class="prettyprint lang-c">    int ret;</pre></td>
</tr>
<tr id="line191">
<td class="line-numbers"><a href="#line191" name="line191">191</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line192">
<td class="line-numbers"><a href="#line192" name="line192">192</a></td>
<td class="code"><pre class="prettyprint lang-c">    if ((ret = avfilter_request_frame(outlink-&gt;src-&gt;inputs[0]))){</pre></td>
</tr>
<tr id="line193">
<td class="line-numbers"><a href="#line193" name="line193">193</a></td>
<td class="code"><pre class="prettyprint lang-c">        if (ret == AVERROR_EOF) {</pre></td>
</tr>
<tr id="line194">
<td class="line-numbers"><a href="#line194" name="line194">194</a></td>
<td class="code"><pre class="prettyprint lang-c">            /* drain cached samples */</pre></td>
</tr>
<tr id="line195">
<td class="line-numbers"><a href="#line195" name="line195">195</a></td>
<td class="code"><pre class="prettyprint lang-c">            while (1) {</pre></td>
</tr>
<tr id="line196">
<td class="line-numbers"><a href="#line196" name="line196">196</a></td>
<td class="code"><pre class="prettyprint lang-c">                outsamples =</pre></td>
</tr>
<tr id="line197">
<td class="line-numbers"><a href="#line197" name="line197">197</a></td>
<td class="code"><pre class="prettyprint lang-c">                    avfilter_get_audio_buffer(outlink, AV_PERM_WRITE, out_nb_samples);</pre></td>
</tr>
<tr id="line198">
<td class="line-numbers"><a href="#line198" name="line198">198</a></td>
<td class="code"><pre class="prettyprint lang-c">                ret = effect-&gt;handler.drain(sox-&gt;effect,</pre></td>
</tr>
<tr id="line199">
<td class="line-numbers"><a href="#line199" name="line199">199</a></td>
<td class="code"><pre class="prettyprint lang-c">                                            (int32_t *)outsamples-&gt;data[0], &amp;out_nb_samples);</pre></td>
</tr>
<tr id="line200">
<td class="line-numbers"><a href="#line200" name="line200">200</a></td>
<td class="code"><pre class="prettyprint lang-c">                outsamples-&gt;audio-&gt;nb_samples = out_nb_samples / effect-&gt;out_signal.channels;</pre></td>
</tr>
<tr id="line201">
<td class="line-numbers"><a href="#line201" name="line201">201</a></td>
<td class="code"><pre class="prettyprint lang-c">                avfilter_filter_samples(outlink, outsamples);</pre></td>
</tr>
<tr id="line202">
<td class="line-numbers"><a href="#line202" name="line202">202</a></td>
<td class="code"><pre class="prettyprint lang-c">                if (ret == SOX_EOF)</pre></td>
</tr>
<tr id="line203">
<td class="line-numbers"><a href="#line203" name="line203">203</a></td>
<td class="code"><pre class="prettyprint lang-c">                    break;</pre></td>
</tr>
<tr id="line204">
<td class="line-numbers"><a href="#line204" name="line204">204</a></td>
<td class="code"><pre class="prettyprint lang-c">            }</pre></td>
</tr>
<tr id="line205">
<td class="line-numbers"><a href="#line205" name="line205">205</a></td>
<td class="code"><pre class="prettyprint lang-c">        }</pre></td>
</tr>
<tr id="line206">
<td class="line-numbers"><a href="#line206" name="line206">206</a></td>
<td class="code"><pre class="prettyprint lang-c">    }</pre></td>
</tr>
<tr id="line207">
<td class="line-numbers"><a href="#line207" name="line207">207</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line208">
<td class="line-numbers"><a href="#line208" name="line208">208</a></td>
<td class="code"><pre class="prettyprint lang-c">    return 0;</pre></td>
</tr>
<tr id="line209">
<td class="line-numbers"><a href="#line209" name="line209">209</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line210">
<td class="line-numbers"><a href="#line210" name="line210">210</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line211">
<td class="line-numbers"><a href="#line211" name="line211">211</a></td>
<td class="code"><pre class="prettyprint lang-c">static void filter_samples(AVFilterLink *inlink, AVFilterBufferRef *insamples)</pre></td>
</tr>
<tr id="line212">
<td class="line-numbers"><a href="#line212" name="line212">212</a></td>
<td class="code"><pre class="prettyprint lang-c">{</pre></td>
</tr>
<tr id="line213">
<td class="line-numbers"><a href="#line213" name="line213">213</a></td>
<td class="code"><pre class="prettyprint lang-c">    SoxContext *sox = inlink-&gt;dst-&gt;priv;</pre></td>
</tr>
<tr id="line214">
<td class="line-numbers"><a href="#line214" name="line214">214</a></td>
<td class="code"><pre class="prettyprint lang-c">    AVFilterBufferRef *outsamples;</pre></td>
</tr>
<tr id="line215">
<td class="line-numbers"><a href="#line215" name="line215">215</a></td>
<td class="code"><pre class="prettyprint lang-c">    sox_effect_t *effect = sox-&gt;effect;</pre></td>
</tr>
<tr id="line216">
<td class="line-numbers"><a href="#line216" name="line216">216</a></td>
<td class="code"><pre class="prettyprint lang-c">    size_t in_nb_samples, out_nb_samples;</pre></td>
</tr>
<tr id="line217">
<td class="line-numbers"><a href="#line217" name="line217">217</a></td>
<td class="code"><pre class="prettyprint lang-c">    int ret;</pre></td>
</tr>
<tr id="line218">
<td class="line-numbers"><a href="#line218" name="line218">218</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line219">
<td class="line-numbers"><a href="#line219" name="line219">219</a></td>
<td class="code"><pre class="prettyprint lang-c">    // FIXME not handling planar data</pre></td>
</tr>
<tr id="line220">
<td class="line-numbers"><a href="#line220" name="line220">220</a></td>
<td class="code"><pre class="prettyprint lang-c">    in_nb_samples = insamples-&gt;audio-&gt;nb_samples;</pre></td>
</tr>
<tr id="line221">
<td class="line-numbers"><a href="#line221" name="line221">221</a></td>
<td class="code"><pre class="prettyprint lang-c">    outsamples = avfilter_get_audio_buffer(inlink, AV_PERM_WRITE, in_nb_samples);</pre></td>
</tr>
<tr id="line222">
<td class="line-numbers"><a href="#line222" name="line222">222</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line223">
<td class="line-numbers"><a href="#line223" name="line223">223</a></td>
<td class="code"><pre class="prettyprint lang-c">    out_nb_samples =</pre></td>
</tr>
<tr id="line224">
<td class="line-numbers"><a href="#line224" name="line224">224</a></td>
<td class="code"><pre class="prettyprint lang-c">    in_nb_samples  = in_nb_samples * effect-&gt;out_signal.channels;</pre></td>
</tr>
<tr id="line225">
<td class="line-numbers"><a href="#line225" name="line225">225</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line226">
<td class="line-numbers"><a href="#line226" name="line226">226</a></td>
<td class="code"><pre class="prettyprint lang-c">    //FIXME not handling cases where not all the input is consumed</pre></td>
</tr>
<tr id="line227">
<td class="line-numbers"><a href="#line227" name="line227">227</a></td>
<td class="code"><pre class="prettyprint lang-c">    ret = effect-&gt;handler.flow(effect,</pre></td>
</tr>
<tr id="line228">
<td class="line-numbers"><a href="#line228" name="line228">228</a></td>
<td class="code"><pre class="prettyprint lang-c">                               (int32_t *)insamples -&gt;data[0],</pre></td>
</tr>
<tr id="line229">
<td class="line-numbers"><a href="#line229" name="line229">229</a></td>
<td class="code"><pre class="prettyprint lang-c">                               (int32_t *)outsamples-&gt;data[0],</pre></td>
</tr>
<tr id="line230">
<td class="line-numbers"><a href="#line230" name="line230">230</a></td>
<td class="code"><pre class="prettyprint lang-c">                               &amp;in_nb_samples, &amp;out_nb_samples);</pre></td>
</tr>
<tr id="line231">
<td class="line-numbers"><a href="#line231" name="line231">231</a></td>
<td class="code"><pre class="prettyprint lang-c">    if (ret == SOX_EOF)</pre></td>
</tr>
<tr id="line232">
<td class="line-numbers"><a href="#line232" name="line232">232</a></td>
<td class="code"><pre class="prettyprint lang-c">        ; /* use drain API */</pre></td>
</tr>
<tr id="line233">
<td class="line-numbers"><a href="#line233" name="line233">233</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line234">
<td class="line-numbers"><a href="#line234" name="line234">234</a></td>
<td class="code"><pre class="prettyprint lang-c">    avfilter_copy_buffer_ref_props(outsamples, insamples);</pre></td>
</tr>
<tr id="line235">
<td class="line-numbers"><a href="#line235" name="line235">235</a></td>
<td class="code"><pre class="prettyprint lang-c">    outsamples-&gt;audio-&gt;nb_samples = out_nb_samples / effect-&gt;out_signal.channels;</pre></td>
</tr>
<tr id="line236">
<td class="line-numbers"><a href="#line236" name="line236">236</a></td>
<td class="code"><pre class="prettyprint lang-c">    avfilter_filter_samples(inlink-&gt;dst-&gt;outputs[0], outsamples);</pre></td>
</tr>
<tr id="line237">
<td class="line-numbers"><a href="#line237" name="line237">237</a></td>
<td class="code"><pre class="prettyprint lang-c">}</pre></td>
</tr>
<tr id="line238">
<td class="line-numbers"><a href="#line238" name="line238">238</a></td>
<td class="code"><pre class="prettyprint lang-c"></pre></td>
</tr>
<tr id="line239">
<td class="line-numbers"><a href="#line239" name="line239">239</a></td>
<td class="code"><pre class="prettyprint lang-c">AVFilter avfilter_af_sox = {</pre></td>
</tr>
<tr id="line240">
<td class="line-numbers"><a href="#line240" name="line240">240</a></td>
<td class="code"><pre class="prettyprint lang-c">    .name          = &quot;sox&quot;,</pre></td>
</tr>
<tr id="line241">
<td class="line-numbers"><a href="#line241" name="line241">241</a></td>
<td class="code"><pre class="prettyprint lang-c">    .description   = NULL_IF_CONFIG_SMALL(&quot;Apply SoX effects.&quot;),</pre></td>
</tr>
<tr id="line242">
<td class="line-numbers"><a href="#line242" name="line242">242</a></td>
<td class="code"><pre class="prettyprint lang-c">    .priv_size     = sizeof(SoxContext),</pre></td>
</tr>
<tr id="line243">
<td class="line-numbers"><a href="#line243" name="line243">243</a></td>
<td class="code"><pre class="prettyprint lang-c">    .init          = init,</pre></td>
</tr>
<tr id="line244">
<td class="line-numbers"><a href="#line244" name="line244">244</a></td>
<td class="code"><pre class="prettyprint lang-c">    .uninit        = uninit,</pre></td>
</tr>
<tr id="line245">
<td class="line-numbers"><a href="#line245" name="line245">245</a></td>
<td class="code"><pre class="prettyprint lang-c">    .query_formats = query_formats,</pre></td>
</tr>
<tr id="line246">
<td class="line-numbers"><a href="#line246" name="line246">246</a></td>
<td class="code"><pre class="prettyprint lang-c">    .inputs    = (const AVFilterPad[]) {</pre></td>
</tr>
<tr id="line247">
<td class="line-numbers"><a href="#line247" name="line247">247</a></td>
<td class="code"><pre class="prettyprint lang-c">        { .name             = &quot;default&quot;,</pre></td>
</tr>
<tr id="line248">
<td class="line-numbers"><a href="#line248" name="line248">248</a></td>
<td class="code"><pre class="prettyprint lang-c">          .type             = AVMEDIA_TYPE_AUDIO,</pre></td>
</tr>
<tr id="line249">
<td class="line-numbers"><a href="#line249" name="line249">249</a></td>
<td class="code"><pre class="prettyprint lang-c">          .filter_samples   = filter_samples,</pre></td>
</tr>
<tr id="line250">
<td class="line-numbers"><a href="#line250" name="line250">250</a></td>
<td class="code"><pre class="prettyprint lang-c">          .min_perms        = AV_PERM_READ },</pre></td>
</tr>
<tr id="line251">
<td class="line-numbers"><a href="#line251" name="line251">251</a></td>
<td class="code"><pre class="prettyprint lang-c">        { .name = NULL}},</pre></td>
</tr>
<tr id="line252">
<td class="line-numbers"><a href="#line252" name="line252">252</a></td>
<td class="code"><pre class="prettyprint lang-c">    .outputs   = (const AVFilterPad[]) {</pre></td>
</tr>
<tr id="line253">
<td class="line-numbers"><a href="#line253" name="line253">253</a></td>
<td class="code"><pre class="prettyprint lang-c">        { .name             = &quot;default&quot;,</pre></td>
</tr>
<tr id="line254">
<td class="line-numbers"><a href="#line254" name="line254">254</a></td>
<td class="code"><pre class="prettyprint lang-c">          .type             = AVMEDIA_TYPE_AUDIO,</pre></td>
</tr>
<tr id="line255">
<td class="line-numbers"><a href="#line255" name="line255">255</a></td>
<td class="code"><pre class="prettyprint lang-c">          .config_props     = config_output,</pre></td>
</tr>
<tr id="line256">
<td class="line-numbers"><a href="#line256" name="line256">256</a></td>
<td class="code"><pre class="prettyprint lang-c">          .request_frame    = request_frame, },</pre></td>
</tr>
<tr id="line257">
<td class="line-numbers"><a href="#line257" name="line257">257</a></td>
<td class="code"><pre class="prettyprint lang-c">        { .name = NULL}</pre></td>
</tr>
<tr id="line258">
<td class="line-numbers"><a href="#line258" name="line258">258</a></td>
<td class="code"><pre class="prettyprint lang-c">    },</pre></td>
</tr>
<tr id="line259">
<td class="line-numbers"><a href="#line259" name="line259">259</a></td>
<td class="code"><pre class="prettyprint lang-c">};</pre></td>
</tr>
</table>  
<script type="text/javascript" charset="utf-8">
  (function () {
      if ($("#codeblob tr td.line-numbers:last").text().length < 3500) {
          prettyPrint();
      } else {
          $("#long-file").show().find("a#highlight-anyway").click(function(e){
              prettyPrint();
              e.preventDefault();
          });
      }
  }());
</script>

      </div>
          </div>
    <div id="footer">
      
<div class="powered-by">
  <a href="http://gitorious.org"><img alt="Poweredby" src="/images/../img/poweredby.png?1294322727" title="Powered by Gitorious" /></a></div>
<script type="text/javascript">
var _gaq = _gaq || [];
_gaq.push(['_setAccount', 'UA-52238-3']);
_gaq.push(['_setDomainName', '.gitorious.org'])
_gaq.push(['_trackPageview']);
(function() {
   var ga = document.createElement('script'); ga.type = 'text/javascript'; ga.async = true;
   ga.src = ('https:' == document.location.protocol ? 'https://ssl' : 'http://www') + '.google-analytics.com/ga.js';
   (document.getElementsByTagName('head')[0] || document.getElementsByTagName('body')[0]).appendChild(ga);
})();
</script><script src="/javascripts/onload.js?1335784269" type="text/javascript"></script>
      
<div id="footer-links">
  <h3>Gitorious</h3>
  <ul>
    <li><a href="/">Home</a></li>
    <li><a href="/about">About</a></li>
    <li><a href="/about/faq">FAQ</a></li>
    <li><a href="/contact">Contact</a></li>
  </ul>
  
    <ul>
      <li><a href="http://groups.google.com/group/gitorious">Discussion group</a></li>
      <li><a href="http://blog.gitorious.org">Blog</a></li>
    </ul>
  
      
<ul>
  <li><a href="http://en.gitorious.org/tos">Terms of Service</a></li>
  <li><a href="http://en.gitorious.org/privacy_policy">Privacy Policy</a></li>
</ul>

  
  
    <ul>
      
        <li><a href="http://gitorious.com/">Professional Gitorious Services</a></li>
      
    </ul>
  
</div>
      <p class="footer-blurb">
  
    <a href="http://gitorious.com">Professional Gitorious services</a> - Git
    hosting at your company, custom features, support and more.
    <a href="http://gitorious.com">gitorious.com</a>.
  
</p>

      <div class="clear"></div>
    </div>
  </div>
</body>
</html>

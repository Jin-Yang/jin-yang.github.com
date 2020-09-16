---
layout: tools
title: Tools
---
<div class="jumbotron">
<h1>时间戳</h1>
<p>Unix Timestamp 是一种时间表示方式，定义为从格林威治时间1970年01月01日00时00分00秒起至现在的总秒数，不仅在Unix类系统中使用，在许多其它操作系统中也被广泛采用。</p>
</div>




<!--
<p><span>现在</span><a id="text_current_timestamp" href="javascript:;">-</a> 控制：<a id="js_timer_start" href="javascript:;"><i
                    class="fas fa-play"></i> 开始</a> <a id="js_timer_stop" href="javascript:;" style="color: #E74C3C;"><i class="fas fa-stop"></i> 停止</a>
        </p>
-->

<p>
<label>时间戳</label>
  <input type="text" class="text" id="input_timestamp" name="timestamp">
  <select id="input_timestamp_unit" name="timestamp_unit">
    <option value="ms">毫秒(ms)</option>
    <option value="s">秒(s)</option>
  </select>
  <button type="button" onclick="test()">转换</button>
  <input type="text" readonly="readonly" id="output_timestamp">
<span>北京时间</span>
</p>


<p>
<label class="form_left">时间</label>
<input type="text" class="text" id="js_datetime_o" name="datetime_o"> <span>北京时间</span>
<button id="js_convert_datetime" type="button">转换</button>
<input type="text" class="text" id="js_timestamp_o" name="timestamp_o">
<select id="js_timestamp_unit_o" name="timestamp_unit_o">
<option value="s">秒(s)</option>
<option value="ms">毫秒(ms)</option>
</select>
</p>


<p>当前时间戳(Unix Timestamp): <input type="text" readonly="readonly" id="text_current_timestamp" /></p>

<script type="text/javascript">
  var getDate = function(ts) {
    var d = new Date(ts);
  };
  var curr = new Date().getTime();
  var timer = setInterval(function() {
    document.getElementById("text_current_timestamp").value=new Date().getTime();
  }, 1000);
  document.getElementById("text_current_timestamp").value=curr;
  document.getElementById("input_timestamp").value=new Date().getTime();
  function test(){
      var output = document.getElementById("output_timestamp");
      var unit = document.getElementById("input_timestamp_unit").value;
      var input = parseInt(document.getElementById("input_timestamp").value);
      if (unit == "s") {
        input = input * 1000;
      }
      var value = new Date(input);
      ////console.log(jutils.formatDate(new Date(1600187681111),"YYYY-MM-DD HH:ii:ss"));
      output.value = value.toISOString();
      //console.log(value.toString());
      //console.log(value.toUTCString());
      //console.log(value.toISOString());
      //console.log(value.toGMTString());
      //console.log(value.toDateString());
      //console.log(value.toTimeString());
  }
</script>

通过 new Date().getTime() 获取时间戳，单位是毫秒。

```
xxxxxx
```

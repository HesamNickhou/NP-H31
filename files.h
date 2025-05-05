static const char* serverIndex PROGMEM =
"<div style='float:left;height:30%;margin-bottom:-300px;'></div>"
"<div style='clear:both;width:90%;height:50%;position:relative;margin:auto;padding:10px;border:2px solid #7593A4;border-radius:12px;;'>"
"<form method='POST' enctype='multipart/form-data' id='upload_form' style='padding-top:70px;'>"
"<input type='file' accept='.bin' name='update' style='margin-top:20px;width:100%;height:120px;font-size:70px;'>"
"<input type='submit' value='Update' style='margin-top:300px;width:100%;height:120px;font-size:70px;color:red;'id='set'>"
"</form></div>"
"<script>const form=document.getElementById('upload_form');const data=new FormData(form);form.addEventListener('submit',async(event)=>{event.preventDefault();event.target.disabled=true;let response=await fetch('/',{method:'POST',body:data'});let result=await response.json();console.log(result);'});</script><script>document.getElementById('set').addEventListener('click',event=>setTimeout(()=>event.target.disabled=true,0));</script>";

static const char* server_success PROGMEM = "<html><body style='height:100%;width:100%;display:flex;justify-content:center;align-items:center;align-content:center;'><div style='text-align:center;font-size:120px;color:green;'>Finished</div></body></html>";
static const char* server_failed  PROGMEM = "<html><body style='height:100%;width:100%;display:flex;justify-content:center;align-items:center;align-content:center;'><div style='text-align:center;font-size:120px;color:red;'>Failed</div></body></html>";
static const char* server_badfile PROGMEM = "<html><body style='height:100%;width:100%;display:flex;justify-content:center;align-items:center;align-content:center;'><div style='text-align:center;font-size:120px;color:red;'>Bad file format</div></body></html>";
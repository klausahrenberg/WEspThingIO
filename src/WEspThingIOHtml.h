#ifndef ESP_THING_HTML_PAGES_H
#define ESP_THING_HTML_PAGES_H

const static char HTTP_COMBO_BOX_FUNCTION_SCRIPT[] PROGMEM = R"=====(
	<script>		
		function getComboA(cb) {
			var value = cb.value;  
			var ma = document.getElementById('ma');			
			ma.style.display = (cb.value=='1' || cb.value=='2' ? 'block' : 'none');
			var wa = document.getElementById('wa');
			wa.style.display = (cb.value=='2' ? 'block' : 'none');
			var na = document.getElementById('na');
			na.style.display = (cb.value!='0' && cb.value!='1' && cb.value!='2' ? 'block' : 'none');
		}
	</script>
)=====";

const static char HTTP_COMBOBOX_OPTION_BEGIN[]         PROGMEM = R"=====(
        <div>
			%s<br>
        	<select class='ip' name='%s' onchange='getComboA(this)'>
)=====";

const static char HTTP_PROPERTY_VISIBILITY[]         PROGMEM = R"=====(
        propVisibility
)=====";

#endif
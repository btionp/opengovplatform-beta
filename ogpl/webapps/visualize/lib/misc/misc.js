/*
 * Removes script from input string
 */

function DVE_replace_script(text) {
	var SCRIPT_REGEX = /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi;
	
	while (SCRIPT_REGEX.test(text)) {
	    text = text.replace(SCRIPT_REGEX, "");
	}
	
	return text;
}

function DVE_test_blacklist(input) {
	var SCRIPT_REGEX = /<script\b[^<]*(?:(?!<\/script>)<[^<]*)*<\/script>/gi;
	if(SCRIPT_REGEX.exec(input)) {
		return true;
	}
	return false;
}

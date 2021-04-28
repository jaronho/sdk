/***********************************************************************
 ** Author:	jaron.ho
 ** Date:	2015-08-12
 ** Brief:	RC4 algorithm
 ***********************************************************************/
function rc4(data, key) {
    var seq = Array(256);           // int
    var das = Array(data.length);   // code of data
    for (var i = 0; i < 256; i++) {
        seq[i] = i;
        var j = (j + seq[i] + key.charCodeAt(i % key.length)) % 256;
        var temp = seq[i];
        seq[i] = seq[j];
        seq[j] = temp;
    }
    for (var i = 0; i < data.length; i++) {
        das[i] = data.charCodeAt(i)
    }
    for (var x = 0; x < das.length; x++) {
        var i = (i + 1) % 256;
        var j = (j + seq[i]) % 256;
        var temp = seq[i];
        seq[i] = seq[j];
        seq[j] = temp;
        var k = (seq[i] + (seq[j] % 256)) % 256;
        das[x] = String.fromCharCode(das[x] ^ seq[k]);
    }
    return das.join('');
}
//----------------------------------------------------------------------
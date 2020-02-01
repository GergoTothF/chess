
var wasmMemory = new WebAssembly.Memory({initial: 16384, maximum: 16384});
var isLoaded = false


// Converts a string to an ASCII byte array on the specified memory
// function stringToASCIIArray(str, outU8Array, idx){
    // let length = str.length + 1;

    // let i;
    // for(i=0; i<length; i++){
        // outU8Array[idx+i] = str.charCodeAt(i);
    // }
    // outU8Array[idx+i]=0;

    // return (idx + length);
// }

// function utf8ToString(h, p) {
  // let s = "";
  // for (i = p; h[i]; i++) {
    // s += String.fromCharCode(h[i]);
  // }
  // return s;
// }

// function decodeString(memory, ptr) {
  // const array = new Uint8Array(memory.buffer, ptr);
  // function* readUntilNul() {
    // let offset = 0;
    // //while (array[offset] !== 0) {
    // while (offset < 20) {
      // if (array[offset] === undefined) {
        // throw new Error(`String ${ptr} continued into undefined memory at ${ptr+offset}`);
      // }
      // yield array[offset];
      // offset++;
    // }
	// console.log(offset);
  // }

  // const bytes = new Uint8Array(readUntilNul());
  // return (new TextDecoder('utf-8')).decode(bytes);
//}

// var importObject = { 
        // 'env': { 
          // '__memory_base': 0,
          // '__table_base': 0,
          // 'memory': wasmMemory,
          // 'table': new WebAssembly.Table({initial: 2, element: 'anyfunc', maximum: 2}),
		  // _abort:  _ => { throw new Error('overflow'); },
		  // abort:  _ => { throw new Error('overflow'); },
		  // _llvm_stacksave: function() {},
		  // _llvm_stackrestore: function() {},
		  // _getenv: function() {},
		  // ___unlock: function() {},
		  // ___setErrNo: function() {},
		  // _emscripten_get_heap_size: function() {},
		  // _clock_gettime: function() {},
		  // ___syscall91: function() {},
		  // ___map_file: function() {},
		  // ___lock: function() {},
		  // abortOnCannotGrowMemory: function() {},
		  // _strftime_l: function() {},
		  // _pthread_cond_wait: function() {},
		  // _emscripten_resize_heap: function() {},
		  // _emscripten_memcpy_big: function() {},
		  // ___cxa_uncaught_exceptions: function() {},
		  // DYNAMICTOP_PTR: 0,
		  // _nanosleep: function() {},
		  // b: function() {},
		  // c: function() {},
		  // d: function() {},
		  // e: function() {},
		  // f: function() {},
		  // g: function() {},
		  // h: function() {},
		  // i: function() {},
		  // j: function() {},
		  // k: function() {},
		  // l: function() {},
		  // m: function() {},
		  // n: function() {},
		  // o: function() {},
		  // p: function() {},
		  // q: function() {},
		  // r: function() {},
		  // s: function() {},
		  // ilyen_nincs_is_megis_mukodik: function() {},
		  // a: 0,
		  // _time: function() {},
		  // _ctime: function() {},
		  // ___buildEnvironment() {},
		  // _asctime: function() {},
		  // _localtime: function() {},
		  // ___syscall146: function() {},
		  // ___syscall54: function() {},
		  // ___syscall145: function() {},
		  // ___syscall140: function() {},
		  // ___syscall6: function() {}
        // },
		// 'global': {
		  // NaN:0,
		  // Infinity:10000
		// }
      // };

var js_wrapped_searched_function

search = function(myStrValue, startTime) {
	var result = js_wrapped_searched_function(myStrValue);
	console.log("The result of search is:", result);
		
	var endTime = new Date();
	var timeDiff = endTime - startTime; //in ms
 
	console.log(" zzzz " + timeDiff); 
	self.postMessage({res: result, tim: timeDiff});
}

onmessage = function(e) {
	var startTime = new Date();
	var myStrValue = e.data;
	
	
	if (isLoaded === false) {
	    self.importScripts('win-chesser-glue.js');
	
		WinChesserModule().then(function(Module) {
			js_wrapped_searched_function = Module.cwrap("search", "string", ["string", "number"]);
			search(myStrValue, startTime)
		});
	    
		isLoaded = true
    }
	else {
	    search(myStrValue, startTime)
	}
	
    console.log(" search ended ");
 };

import * as fs from "fs";

import { mt63Source } from "../dist/mt63-wasm.js";
import { MT63 } from "../dist/mt63.js"

const mt63 = new MT63( mt63Source );

const data = mt63.encode( "MT63 Test!", 2000, 2000, 1 );

fs.writeFileSync( './encoded.raw', data );

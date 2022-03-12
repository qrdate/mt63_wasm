import * as fs from "fs";

import { source } from "../dist/mt63-tx-source.js";
import { MT63Tx } from "../dist/MT63-tx.js"

const mt63 = new MT63Tx( source );

const data = mt63.transmit( "MT63 Test!", 2000, 2000, 1 );

fs.writeFileSync( './encoded.raw', data );

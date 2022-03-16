import * as fs from 'fs';

import { MT63Rx } from '../dist/MT63-rx.js';
import { source } from '../dist/mt63-rx-source.js';

const mt63 = new MT63Rx(source, 2000, 2000, 1, 48000);

const data = fs.readFileSync('./encoded.raw');
const samples = new Float32Array(data.buffer);

let text = '';

// There is a limit to the amount of audio the receiver con process at a time which seems to be
// 7824 samples, so we should keep it well below that.
const CHUNK_SIZE = 4096;

for (let i = 0; i < samples.length; i += CHUNK_SIZE) {
    text += mt63.receive(samples.slice(i, i + CHUNK_SIZE));
}

// After processing all the audio we need to flush all the internal buffers to get
// the last bit of data out.
text += mt63.flush();

console.log('Received: ' + text);

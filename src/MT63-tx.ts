import { TinyWASI } from 'tinywasi';

export class MT63Tx {
    private tinyWASI: TinyWASI = new TinyWASI();
    private instance: WebAssembly.Instance;

    constructor(source: BufferSource) {
        const mod = new WebAssembly.Module(source);
        this.instance = new WebAssembly.Instance(mod, this.tinyWASI.imports);
        this.tinyWASI.initialize(this.instance);
    }

    transmit(
        text: string,
        center: number,
        bandwidth: 500 | 1000 | 2000,
        interleave: 0 | 1,
    ): Float32Array {
        const getInputBuffer = this.instance.exports.getInputBuffer as CallableFunction;
        const transmit = this.instance.exports.transmit as CallableFunction;
        const getOutputBuffer = this.instance.exports.getOutputBuffer as CallableFunction;

        const data = new TextEncoder().encode(text);

        const textPtr = getInputBuffer(data.byteLength);
        const memory = new Uint8Array(this.getMemory().buffer, textPtr, data.byteLength);
        memory.set(data);

        const audioLength = transmit(center, bandwidth, interleave);

        if (audioLength === -1) throw new Error('Encoding Failed!');

        const audioPtr = getOutputBuffer();
        const audio = new Float32Array(this.getMemory().buffer, audioPtr, audioLength);

        return audio.subarray();
    }

    private getMemory(): WebAssembly.Memory {
        return this.instance.exports.memory as WebAssembly.Memory;
    }
}

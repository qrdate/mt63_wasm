import { TinyWASI } from 'tinywasi';

export class MT63Rx {
    private tinyWASI: TinyWASI = new TinyWASI();
    private instance: WebAssembly.Instance;

    constructor(
        source: BufferSource,
        center: number,
        bandwidth: 500 | 1000 | 2000,
        interleave: 0 | 1,
        sampleRate = 8000,
        squelch = 10.0,
    ) {
        const mod = new WebAssembly.Module(source);
        this.instance = new WebAssembly.Instance(mod, this.tinyWASI.imports);
        this.tinyWASI.initialize(this.instance);

        const setSampleRate = this.instance.exports.setSampleRate as CallableFunction;
        setSampleRate(sampleRate);

        const initRx = this.instance.exports.initRx as CallableFunction;
        initRx(center, bandwidth, interleave, squelch);
    }

    receive(samples: Float32Array): string {
        const getInputBuffer = this.instance.exports.getInputBuffer as CallableFunction;
        const receive = this.instance.exports.receive as CallableFunction;
        const getOutputBuffer = this.instance.exports.getOutputBuffer as CallableFunction;

        const audioPtr = getInputBuffer(samples.length);
        const memory = new Float32Array(this.getMemory().buffer, audioPtr, samples.length);
        memory.set(samples);

        const dataLength = receive();

        const dataPtr = getOutputBuffer();
        const data = new Uint8Array(this.getMemory().buffer, dataPtr, dataLength);

        const text = new TextDecoder('UTF-8').decode(data);

        return text;
    }

    flush(): string {
        const flush = this.instance.exports.flush as CallableFunction;
        const getOutputBuffer = this.instance.exports.getOutputBuffer as CallableFunction;

        const dataLength = flush();

        const dataPtr = getOutputBuffer();
        const data = new Uint8Array(this.getMemory().buffer, dataPtr, dataLength);

        const text = new TextDecoder('UTF-8').decode(data);

        return text;
    }

    getSNR(): number {
        const getSNR = this.instance.exports.getSNR as CallableFunction;
        return getSNR();
    }

    getLock(): number {
        const getLock = this.instance.exports.getLock as CallableFunction;
        return getLock();
    }

    private getMemory(): WebAssembly.Memory {
        return this.instance.exports.memory as WebAssembly.Memory;
    }
}

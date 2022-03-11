import { TinyWASI } from "tinywasi";

export class MT63
{
	private tinyWASI: TinyWASI = new TinyWASI();
	private instance: WebAssembly.Instance;

	constructor( source: BufferSource )
	{
		const mod = new WebAssembly.Module( source );
		this.instance = new WebAssembly.Instance( mod, this.tinyWASI.imports );
		this.tinyWASI.initialize( this.instance );
	}

	encode( text: string, center: number, bandwidth: 500 | 1000 | 2000, interleave: 0 | 1 ): Float32Array
	{
		const getEncodeText = this.instance.exports.getEncodeText as CallableFunction;
		const encode = this.instance.exports.encode as CallableFunction;
		const getEncodeAudio = this.instance.exports.getEncodeAudio as CallableFunction;

		const data = new TextEncoder().encode( text );

		const textPtr = getEncodeText( data.byteLength );
		const memory = new Uint8Array( this.getMemory().buffer, textPtr, data.byteLength );
		memory.set( data );

		const audioLength = encode( center, bandwidth, interleave );

		if( audioLength == 0 )
			throw new Error( "Encoding Failed!" );

		const audioPtr = getEncodeAudio();
		const audio = new Float32Array( this.getMemory().buffer, audioPtr, audioLength );

		return audio.subarray();
	}

	private getMemory(): WebAssembly.Memory
	{
		return ( this.instance.exports.memory as WebAssembly.Memory );
	}
}

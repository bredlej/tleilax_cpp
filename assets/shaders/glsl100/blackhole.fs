#version 100

precision mediump float;
uniform float time; // time
//uniform vec2  resolution; // resolution
varying vec4 fragColor;
varying vec3 fragPosition;
void main(void){
    //float time = 1.0;
    vec2 resolution = vec2(40.0, 40.0);
	vec3 destColor = vec3(fragColor.r, fragColor.g, fragColor.b);
	vec2 p = (fragPosition.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
	float a = atan(p.y / p.x) * 2.0; // Instead of * 2.0, try * 26 or * 128 and higher
	float l = 0.05 / abs(length(p) - 0.8 + sin(a + time * 4.5) * 0.1);
	destColor *= 1.9+ cos(a + time * 00.13) * 0.03;

	vec3 destColor2 = vec3(0.0, 0.2, 0.9);
	vec2 p2 = (fragPosition.xy * 3.0 - resolution) / min(resolution.x, resolution.y);
	float a2 = atan(p.y / p.x) * 3.0;
	float l2 = 0.09 / abs(length(p) + 0.1 - (tan(time/2.)+0.5) + sin(a + time * 13.5) * (0.1 * l));
	destColor2 *= ( 0.5 + sin(a + time * 00.03) * 0.03 ) * 4.0;

	vec3 destColor3 = vec3(0.2, 0.9, 0.35);
	vec2 p3 = (fragPosition.xy * 2.0 - resolution) / min(resolution.x, resolution.y);
	float a3 = atan(p.y / p.x) * 100.0;
	float l3 = 0.05 / abs(length(p) - 0.4 + sin(a + time * 23.5) * (0.1 * l2));
	destColor3 *= 1.5 + sin(a + time * 10.23) * 0.03;

	gl_FragColor = vec4(l*destColor + l2*destColor2 + l3*destColor3 * time, 1.0);
}
/* gui/texture.fsh */
#if __VERSION__ < 130
#define in varying
#define outPixelColor gl_FragColor
#else
out vec4 outPixelColor;
#endif
uniform sampler2D mainTexture;
in vec2 outTexCoord;

void main() {
    vec4 color = texture2D(mainTexture, outTexCoord);
    outPixelColor = color;
}


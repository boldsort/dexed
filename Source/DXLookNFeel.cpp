/**
 *
 * Copyright (c) 2013 Pascal Gauthier.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */
 
#include "DXLookNFeel.h"
#include "PluginProcessor.h"

/**
 * Algorithm arrangements, based on the DX1 display.
 */
static const char algoArr[][13] = {
    // 1  2  3  4  5   6   7   8   9   A   B   C   D  
    {  0, 0, 1, 3, 0,  0,  0,  2,  4,  0,  0,  5, -6 },  // 1
    {  0, 0, 1, 3, 0,  0,  0, -2,  4,  0,  0,  5,  6 },  // 2
    {  0, 0, 1, 4, 0,  0,  0,  2,  5,  0,  3, -6,  0 },  // 3
    {  0, 0, 1, -4, 0,  0,  0,  2,  -5, 0, 3, -6,  0 },  // 4
    {  0, 1, 3, 5, 0,  0,  2,   4,  -6, 0, 0,  0,  0 },  // 5
    {  0, 1, 3, -5, 0,  0, 2,   4,  -6, 0, 0,  0,  0 },  // 6
    {  0, 1, 3, 0, 0,  0, -2,  4,  5,  0,  0,  6,  0 },  // 7
    {  0, 1, 3, 0, 0,  0,  2, -4,  5,  0,  0,  6,  0 },  // 8
    {  0, 1, 3, 0, 0,  0, -2,  4,  5,  0,  0,  6,  0 },  // 9
    {  0, 0, 4, 1, 0,  0,  5,  6,  2,  0,  0, -3,  0 },  // 10
    {  0, 0, 4, 1, 0,  0,  5, -6,  2,  0,  0,  3,  0 },  // 11
    {  0, 0, 3, 0, 1,  0,  4,  5,  6, -2,  0,  0,  0 },  // 12
    {  0, 0, 3, 0, 1,  0,  4,  5, -6,  2,  0,  0,  0 },  // 13
    // 1  2  3  4  5   6   7   8   9   A   B   C   D  
    {  0, 0, 1, 3, 0,  0,  0,  2,  4,  0,  5, -6,  0 },  // 14
    {  0, 0, 1, 3, 0,  0,  0, -2,  4,  0,  5,  6,  0 },  // 15
    {  0, 0, 1, 0, 0,  0,  2,  3,  5,  0,  4, -6,  0 },  // 16
    {  0, 0, 1, 0, 0,  0, -2,  3,  5,  0,  4,  6,  0 },  // 17
    {  0, 0, 1, 0, 0,  0,  2, -3,  4,  0,  0,  5,  6 },  // 18
    {  0, 0, 1, 4, 5,  0,  0,  2, -6,  0,  3,  0,  0 },  // 19
    {  0, 1, 2, 0, 4,  0, -3,  0,  5,  6,  0,  0,  0 },  // 20
    {  0, 1, 2, 4, 5,  0, -3,  0,  6,  0,  0,  0,  0 },  // 21
    {  0, 1, 3, 4, 5,  0,  2,  0, -6,  0,  0,  0,  0 },  // 22
    {  0, 1, 2, 4, 5,  0,  0,  3, -6,  0,  0,  0,  0 },  // 23
    {  1, 2, 3, 4, 5,  0,  0,  0, -6,  0,  0,  0,  0 },  // 24
    {  1, 2, 3, 4, 5,  0,  0,  0, -6,  0,  0,  0,  0 },  // 25
    {  0, 1, 2, 0, 4,  0,  0,  3,  5, -6,  0,  0,  0 },  // 26
    // 1  2  3  4  5   6   7   8   9   A   B   C   D
    {  0, 1, 2, 0, 4,  0,  0, -3,  5,  6,  0,  0,  0 },  // 27
    {  0, 1, 3, 6, 0,  0,  2,  4,  0,  0, -5,  0,  0 },  // 28
    {  0, 1, 2, 3, 5,  0,  0,  0,  4, -6,  0,  0,  0 },  // 29
    {  0, 1, 2, 3, 6,  0,  0,  0,  4,  0,  0, -5,  0 },  // 30
    {  1, 2, 3, 4, 5,  0,  0,  0,  0, -6,  0,  0,  0 },  // 31
    {  1, 2, 3, 4, 5, -6,  0,  0,  0,  0,  0,  0,  0 }   // 32
};

AlgoDisplay::AlgoDisplay() {
    static char tmpAlgo = 0;
    algo = &tmpAlgo;
}

/**
 * For now, this is hardcoded 126x56 (21x14 each)
 */
void AlgoDisplay::paint(Graphics &g) {
    int alg;

    if ( *algo <= 31 ) { 
        alg = *algo;
    } else {
        alg = 31;
    }
    const char *arr = algoArr[alg];

    g.setColour (Colours::black.withAlpha(0.1f));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) getWidth(), (float) getHeight(), 1.0f);
    
    for(int i=0;i<13;i++) {
        int target = arr[i];
        if ( target == 0 )
            continue;
        
        if ( i < 6 ) {
            drawOp(g, i, 3, target);
            continue;
        }
        
        if ( i < 10 ){
            drawOp(g, (i-6)+1, 2, target);
            continue;
        }
        
        if ( i < 12 ) {
            drawOp(g, (i-10)+2, 1, target);
            continue;
        }
    
        // last one
        drawOp(g, (i-12)+3, 0, target);
    }

    String algoTxt;
    algoTxt << (alg+1);
    g.drawText(algoTxt, 5, 1, 21, 14, Justification::left, true);
}

void AlgoDisplay::drawOp(Graphics &g, int x, int y, int num) {
    String txt;
    txt << abs(num);

    int offx = 24;
    int offy = 17;
    
    g.setColour(Colour(0xFF0FC00F));
    g.fillRect(x*offx+4, y*offy+3, offx-2, offy-1);
    g.setColour(Colour(0xFFFFFFFF));
    g.drawText(txt, x*offx+3, y*offy+2, offx+2, offy+2, Justification::centred, true);
    if ( num < 0 ) {
        g.setColour(Colour(0xFFFFFFFF));
        int x1 = (x*offx) + 24;
        g.drawLine(x1+1, y*offy+3, x1+1, y*offy+offy+2, 3);
    }
}

void EnvDisplay::paint(Graphics &g) {
    int rate[4];
    int level[4];
    
    g.setColour(Colours::black.withAlpha(0.1f));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) getWidth(), (float) getHeight(), 1.0f);
    g.setColour(Colours::white);
    
    for (int i = 0; i < 4; i++) {
        rate[i] = s_rate[i]->getValue();
        level[i] = s_level[i]->getValue();
    }
    
    env.init(rate, level, 99 << 5, 0);
    env.keydown(true);
    for (int i = 0; i < 72; i++) {
        int32_t pos = env.getsample();
        for (int j = 0; j < 16; j++) {
            env.getsample();
        }
        g.setPixel(i, 32 - (sqrt(pos) / 512));
    }
    env.keydown(false);
    for (int i = 0; i < 24; i++) {
        int32_t pos = env.getsample();
        for (int j = 0; j < 16; j++) {
            env.getsample();
        }
        g.setPixel(i + 72, 32 - (sqrt(pos) / 512));
    }
}

PitchEnvDisplay::PitchEnvDisplay() {
    static char tmpDisplay[8];
    memset(&tmpDisplay, 0x00, 8);
    pvalues = (char *) &tmpDisplay;
}

void PitchEnvDisplay::paint(Graphics &g) {
    g.setColour(Colours::black.withAlpha(0.1f));
    g.fillRoundedRectangle (0.0f, 0.0f, (float) getWidth(), (float) getHeight(), 1.0f);
    g.setColour(Colours::white);
    
    char *levels = pvalues;
    char *rates = pvalues + 4;
    
    float dist[4];
    float total;

    int old = pitchenv_tab[levels[3]] + 128;
    // find the scale

    for(int i=0;i<4;i++) {
        int nw = pitchenv_tab[levels[i]] + 128;
        dist[i] = ((float)abs(nw - old)) / pitchenv_rate[rates[i]];
        total += dist[i];
        old = nw;
    }
    
    if ( total == 0 ) {
        dist[0] = 1;
        total = 1;
    }
    //TRACE("DISPLAY %f %f %f %f", dist[0], dist[1], dist[2], dist[3]);

    // TODO : this is WIP
    float ratio =  96 / total;

    int oldx = 0;
    int oldy = (pitchenv_tab[levels[3]] + 128) / 10;

    for(int i=0;i<4;i++) {
        int newx = dist[i] * ratio + oldx;
        int newy = (pitchenv_tab[levels[i]] + 128) / 10;

        g.drawLine(oldx, oldy, newx, newy, 2);
        
        oldx = newx;
        oldy = newy;
    }
}


void VuMeter::paint(Graphics &g) {
    
    // taken from the drawLevelMeter ; 
    float width = getWidth();
    float height = getHeight();
    
    g.setColour (Colours::black);
    g.fillRoundedRectangle (0.0f, 0.0f, (float)  width, (float) height, 0);
    /*g.setColour (Colours::black.withAlpha (0.2f));
    g.drawRoundedRectangle (1.0f, 1.0f, width - 2.0f, height - 2.0f, 3.0f, 1.0f);*/
    
    const int totalBlocks = 16;
    const int numBlocks = roundToInt (totalBlocks * v);
    const float h = (height - 6.0f) / (float) totalBlocks;
    
    for (int i = 0; i < totalBlocks; ++i) {
        g.setColour (Colours::red);
        if (i >= numBlocks)
            g.setColour (Colours::red.withAlpha (0.2f));
        else
            g.setColour (Colours::red);
        //g.fillRoundedRectangle (3.0f + i * w + w * 0.1f, 3.0f, w * 0.8f, height - 6.0f, w * 0.4f);
        
        g.fillRoundedRectangle (3.0f, (height-3.0f) - (3.0f + i * h + h * 0.1f) , width - 6.0f, h * 0.8f, 0);
    }
}

DXLookNFeel::DXLookNFeel() {
    setColour(TextButton::buttonColourId,Colour(0xFF0FC00F));
    setColour(Slider::rotarySliderOutlineColourId,Colour(0xFF0FC00F));
    setColour(Slider::rotarySliderFillColourId,Colour(0xFFFFFFFF));
}

window.Edge = (function(){
  function toGray(ctx, w, h){
    const img = ctx.getImageData(0,0,w,h);
    const src = img.data;
    const gray = new Uint8ClampedArray(w*h);
    for(let i=0,j=0;i<src.length;i+=4,++j){
      const r=src[i], g=src[i+1], b=src[i+2];
      gray[j] = (0.299*r + 0.587*g + 0.114*b)|0;
    }
    return gray;
  }

  function sobel(gray, w, h, threshold){
    const out = new Uint8ClampedArray(w*h);
    const idx=(x,y)=>y*w+x;
    for(let y=1;y<h-1;++y){
      for(let x=1;x<w-1;++x){
        const g00=gray[idx(x-1,y-1)], g01=gray[idx(x,y-1)], g02=gray[idx(x+1,y-1)];
        const g10=gray[idx(x-1,y  )], g12=gray[idx(x+1,y  )];
        const g20=gray[idx(x-1,y+1)], g21=gray[idx(x,y+1)], g22=gray[idx(x+1,y+1)];
        const gx = (-g00 + g02) + (-2*g10 + 2*g12) + (-g20 + g22);
        const gy = ( g00 + 2*g01 + g02) + (-g20 - 2*g21 - g22);
        const mag = Math.abs(gx) + Math.abs(gy);
        out[idx(x,y)] = (mag>>3) >= threshold ? 255 : 0;
      }
    }
    return out;
  }

  function getDims(source){
    const isVideo = source && typeof source.videoWidth === 'number' && source.videoWidth > 0;
    const w = isVideo ? source.videoWidth : (source.naturalWidth || source.width || 0);
    const h = isVideo ? source.videoHeight : (source.naturalHeight || source.height || 0);
    return { w, h };
  }

  function drawEdges(ctx, source, threshold=40){
    const { w, h } = getDims(source);
    if (!w || !h) return;
    // Ensure canvas sized appropriately
    if (ctx.canvas.width !== w || ctx.canvas.height !== h) {
      ctx.canvas.width = w; ctx.canvas.height = h;
    }
    ctx.drawImage(source, 0, 0, w, h);
    const gray = toGray(ctx, w, h);
    const edges = sobel(gray, w, h, threshold);
    const out = ctx.createImageData(w,h);
    for(let i=0,j=0;j<edges.length;i+=4,++j){
      const v = edges[j];
      out.data[i]=v; out.data[i+1]=v; out.data[i+2]=v; out.data[i+3]=255;
    }
    ctx.putImageData(out,0,0);
  }

  return { drawEdges };
})();



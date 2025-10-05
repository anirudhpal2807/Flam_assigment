const stats = document.getElementById('stats') as HTMLDivElement;
const frame = document.getElementById('frame') as HTMLImageElement;

// Dummy base64 PNG (1x1 pixel cyan), used if sample.png not found
const tinyPngBase64 = 'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGNgYAAAAAMAASsJTYQAAAAASUVORK5CYII=';

function setStats(fps: number, width: number, height: number) {
  stats.textContent = `FPS: ${fps.toFixed(1)} | Resolution: ${width}x${height}`;
}

function setImage(base64Png: string) {
  frame.src = `data:image/png;base64,${base64Png}`;
}

async function tryLoadSample() {
  try {
    const res = await fetch('sample.png', { cache: 'no-store' });
    if (res.ok) {
      const blob = await res.blob();
      const url = URL.createObjectURL(blob);
      frame.src = url;
      // Attempt to read dimensions once image loads
      await new Promise<void>((resolve) => {
        frame.onload = () => resolve();
        frame.onerror = () => resolve();
      });
      const w = frame.naturalWidth || 640;
      const h = frame.naturalHeight || 480;
      setStats(0, w, h);
      return;
    }
  } catch {}
  // Fallback to tiny placeholder
  setStats(0, 1, 1);
  setImage(tinyPngBase64);
}

tryLoadSample();



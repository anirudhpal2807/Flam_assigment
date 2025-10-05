const stats = document.getElementById('stats') as HTMLDivElement;
const frame = document.getElementById('frame') as HTMLImageElement;

// Dummy base64 PNG (1x1 pixel cyan), will replace with real sample later
const tinyPngBase64 =
  'iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR4nGNgYAAAAAMAASsJTYQAAAAASUVORK5CYII=';

function setStats(fps: number, width: number, height: number) {
  stats.textContent = `FPS: ${fps.toFixed(1)} | Resolution: ${width}x${height}`;
}

function setImage(base64Png: string) {
  frame.src = `data:image/png;base64,${base64Png}`;
}

// Initialize with dummy values
setStats(0, 1, 1);
setImage(tinyPngBase64);

// Simulate FPS updates
let t = 0;
setInterval(() => {
  t += 1;
  const fps = 15 + 2 * Math.sin(t / 5);
  setStats(fps, 640, 480);
}, 500);



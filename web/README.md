# EdgeViewer Web

- Build: `npm install && npm run build`
- Serve: `npm run dev` (serves `dist/`)

Place a `sample.png` in `public/` to display it in the viewer. The page will try to load `sample.png` and fall back to a 1x1 placeholder if not found.

Recommended: Export a processed frame (PNG) from the Android app run and copy it to `web/public/sample.png`, then rebuild.



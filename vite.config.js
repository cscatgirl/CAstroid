import { defineConfig } from "vite";
import { readFileSync, existsSync } from "fs";
import { resolve } from "path";

// Plugin to serve WASM files from build directory during development
function wasmPlugin() {
    return {
        name: 'wasm-dev-plugin',
        configureServer(server) {
            // Serve files from build/ directory at /build/*
            server.middlewares.use((req, res, next) => {
                if (req.url?.startsWith('/build/')) {
                    const filePath = resolve(process.cwd(), req.url.slice(1));
                    if (existsSync(filePath)) {
                        if (filePath.endsWith('.wasm')) {
                            res.setHeader('Content-Type', 'application/wasm');
                        } else if (filePath.endsWith('.js')) {
                            res.setHeader('Content-Type', 'application/javascript');
                        }
                        res.end(readFileSync(filePath));
                        return;
                    }
                }
                next();
            });
        }
    };
}

export default defineConfig({
    publicDir: 'public', // Vite will copy contents of public/ to dist/ during build
    plugins: [wasmPlugin()],
    build: {
        // No special handling needed - public directory files are copied as-is
    },
    server: {
        fs: {
            // Allow serving files from build directory during development
            allow: ['.']
        }
    },
    optimizeDeps: {
        exclude: ['@rcade/sdk'] // Don't pre-bundle the SDK
    }
});

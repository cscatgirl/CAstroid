// Export PluginChannel from SDK so C++ can access it
import { PluginChannel } from "@rcade/sdk";

// Make PluginChannel available globally for C++ WASM to use
window.PluginChannel = PluginChannel;

// Dynamically import the compiled WASM module
// In dev: loads from /build/main.js (served by Vite)
// In production: loads from /main.js in public directory
async function loadWasmModule() {
  try {
    let createModule;

    // Load the WASM module via script tag (not ES module)
    const script = document.createElement("script");
    script.src = import.meta.env.DEV ? "/build/main.js" : "/main.js";
    script.type = "text/javascript";

    await new Promise((resolve, reject) => {
      script.onload = resolve;
      script.onerror = reject;
      document.head.appendChild(script);
    });

    createModule = window.createModule;

    if (!createModule) {
      throw new Error("createModule function not found");
    }

    // Initialize the module
    await createModule();
    console.log("C++ WASM module loaded successfully");
  } catch (err) {
    console.error("Failed to load WASM module:", err);
    document.body.innerHTML = `
            <div style="padding: 20px; color: #f44; font-family: monospace;">
                <h2>Error loading game</h2>
                <pre>${err.message}</pre>
            </div>
        `;
  }
}

loadWasmModule();

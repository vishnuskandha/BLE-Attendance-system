// Syntax check for the Vercel serverless functions.
// Each file is piped into `node --input-type=module --check` so that
// `import`/`export` syntax is validated as ESM regardless of package.json
// module type, without executing any code or resolving dependencies.
import { readFile } from "node:fs/promises";
import { spawnSync } from "node:child_process";
import { fileURLToPath } from "node:url";
import path from "node:path";

const here = path.dirname(fileURLToPath(import.meta.url));
const functions = ["attendance.js", "students.js", "debug-kv.js"];

let failed = false;

for (const file of functions) {
  const abs = path.join(here, "..", "api", file);
  const src = await readFile(abs, "utf8");
  const res = spawnSync(
    process.execPath,
    ["--input-type=module", "--check", "-"],
    { input: src, encoding: "utf8" }
  );

  if (res.status === 0) {
    console.log(`OK api/${file}`);
  } else {
    failed = true;
    console.error(`FAIL api/${file}`);
    process.stderr.write(res.stderr);
  }
}

if (failed) {
  console.error("Validation failed.");
  process.exit(1);
}

console.log("All serverless functions valid.");

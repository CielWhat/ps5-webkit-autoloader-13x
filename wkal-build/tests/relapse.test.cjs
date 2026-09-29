// Host-only integration tests. No browser/kernel exploit is executed.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const root = path.resolve(__dirname, '..');
const read = p => fs.readFileSync(path.join(root, p), 'utf8');
const app = read('frontend/autoloader/app.js');

function element() {
  return { style: {}, classList: { add() {}, remove() {} }, children: [],
    parentNode: {}, appendChild(e) { this.children.push(e); },
    get childElementCount() { return this.children.length; } };
}
function route(fw, force = '') {
  const elements = {};
  const events = {};
  const context = {
    document: { getElementById(id) { return elements[id] ||= element(); }, createElement: element },
    navigator: { userAgent: fw ? 'PlayStation 5/' + fw : 'Desktop' },
    window: { location: { search: force, origin: 'http://localhost' },
      addEventListener(name, fn) { events[name] = fn; } },
    sessionStorage: { setItem() {}, removeItem() {} },
    setTimeout() {}, setInterval() { return 1; }, clearInterval() {}, URLSearchParams,
  };
  vm.runInNewContext(app, context);
  events.load();
  return { elements, events, context };
}
for (const fw of ['13.00', '13.20', '13.40', '13.42', '13.60']) {
  assert.equal(route(fw).elements.exploit.src, 'relapse/index.html?autoload=payload.elf');
  assert.ok(fs.existsSync(path.join(root, 'frontend/autoloader/relapse/offsets', fw + '.js')));
}
for (const fw of ['13.50', '13.61', '14.00', '6.00', null])
  assert.equal(route(fw).elements.exploit.src, 'about:blank');
assert.match(route('5.50').elements.exploit.src, /^umtx2\//);
for (const fw of ['7.00', '9.60', '12.00', '12.60', '12.70']) {
  assert.match(route(fw).elements.exploit.src, /^relapse\//);
  assert.ok(fs.existsSync(path.join(root, 'frontend/autoloader/relapse/offsets', fw + '.js')));
}
// 9.05/11.40 have no Relapse offsets: legacy poops fallback.
assert.match(route('9.05').elements.exploit.src, /^slopkit\/slopkit\/poops/);
assert.match(route('11.40').elements.exploit.src, /^slopkit\/slopkit\/poops/);
assert.match(route('12.60', '?force=p2jb').elements.exploit.src, /^slopkit\/slopkit\/p2jb/);
assert.match(route('13.40', '?force=relapse').elements.exploit.src, /^relapse\//);
const routed = route('13.40');
const before = routed.elements.progressLabel.textContent;
routed.events.message({ source: {}, origin: 'http://localhost',
  data: { type: 'wkal', kind: 'autoload', ok: true } });
assert.equal(routed.elements.progressLabel.textContent, before);

async function offsets() {
  let script;
  const context = { window: { fw_str: '13.40', firmware: { rejection: () => null } },
    document: { createElement: () => ({}), body: { appendChild(s) { script = s; } } } };
  vm.runInNewContext(read('frontend/autoloader/relapse/src/main.js'), context);
  assert.equal(script.src, 'offsets/13.40.js');
  let ready = false;
  context.window.relapseOffsetsReady.then(() => { ready = true; });
  await Promise.resolve();
  assert.equal(ready, false);
  script.onload();
  await context.window.relapseOffsetsReady;
  assert.equal(ready, true);
  const failedContext = { ...context, window: { ...context.window } };
  vm.runInNewContext(read('frontend/autoloader/relapse/src/main.js'), failedContext);
  script.onerror();
  await assert.rejects(failedContext.window.relapseOffsetsReady, /Failed to load offsets/);
}

async function transfer({ badElf = false, failWrite = false, notFound = false } = {}) {
  class Int64 { constructor(low, hi = 0) { this.low = low; this.hi = hi; } add32(n) { return new Int64(this.low + n, this.hi); } }
  const calls = [];
  const bytes = new Uint8Array(0x1004);
  bytes.set(badElf ? [0, 0, 0, 0] : [0x7f, 0x45, 0x4c, 0x46]);
  let fetchPath;
  const context = { int64: Int64, URLSearchParams, Uint8Array,
    window: { location: { search: '?autoload=payload.elf' } },
    fetch: async url => { fetchPath = url; return { ok: !notFound, status: 404, arrayBuffer: async () => bytes.buffer }; },
    SYS_MMAP: 'mmap', SYS_MUNMAP: 'munmap', SYS_SOCKET: 'socket', SYS_CONNECT: 'connect',
    SYS_CLOSE: 'close', SYS_WRITE: 'write', setTimeout,
  };
  const source = read('frontend/autoloader/relapse/src/kexp.js')
    .replace(/^import .*;\n/, '').replaceAll('export async function', 'async function');
  vm.runInNewContext(source, context);
  const p = { malloc: () => new Int64(0x20000), write8() {}, write4() {}, write1() {}, read4: () => 0x464c457f };
  const chain = { async syscall(name, ...args) {
    calls.push([name, ...args]);
    return new Int64(name === 'mmap' ? 0x10000 : name === 'socket' ? 3 :
      name === 'write' ? (failWrite ? -1 : Math.min(1024, args[2])) : 0);
  } };
  const result = context.loadAutoloadPayload(p, chain);
  if (badElf || notFound) {
    await assert.rejects(result, badElf ? /not an ELF/ : /HTTP 404/);
    assert.equal(calls.length, 0);
  } else {
    if (failWrite) await assert.rejects(result, /socket write failed/);
    else {
      assert.equal(await result, bytes.length);
      assert.equal(calls.filter(c => c[0] === 'write').length, 5);
    }
    assert.equal(calls.at(-2)[0], 'close');
    assert.equal(calls.at(-1)[0], 'munmap');
  }
  assert.equal(fetchPath, '../payloads/payload.elf');
}
(async () => {
  await offsets();
  await transfer();
  await transfer({ failWrite: true });
  await transfer({ badElf: true });
  await transfer({ notFound: true });
  console.log('PASS: firmware routing, message isolation, offset readiness, payload handoff and failure cleanup');
})().catch(e => { console.error(e); process.exitCode = 1; });

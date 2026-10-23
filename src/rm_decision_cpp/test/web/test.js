const byId = id => document.getElementById(id);
let state = null;
let busy = false;
const dirty = new Set();
const inputs = new Map();

function render(data) {
  if (!inputs.size) {
    for (const [group, fields] of Object.entries(data)) {
      const heading = document.createElement('h2');
      heading.textContent = group;
      const section = document.createElement('div');
      section.className = 'group';
      byId('fields').append(heading, section);
      for (const [field, value] of Object.entries(fields)) {
        const key = `${group}.${field}`;
        const label = document.createElement('label');
        label.className = 'field';
        const caption = document.createElement('span');
        caption.textContent = field;
        const input = document.createElement('input');
        input.type = typeof value === 'boolean' ? 'checkbox' : 'number';
        input.step = 'any';
        input.addEventListener('input', () => dirty.add(key));
        label.append(caption, input);
        section.append(label);
        inputs.set(key, input);
      }
    }
  }
  for (const [key, input] of inputs) {
    if (dirty.has(key)) continue;
    const [group, field] = key.split('.');
    if (input.type === 'checkbox') input.checked = data[group][field];
    else input.value = data[group][field];
  }
}

async function refresh() {
  const response = await fetch('/api/test/state');
  state = await response.json();
  byId('status').textContent = state.online ? '10 Hz · ' + state.scenario : '节点离线';
  byId('enabled').checked = !!state.web_enabled;
  byId('enabled').disabled = busy || !state.online;
  const disabled = busy || !state.online || !state.web_enabled;
  for (const id of ['fields', 'apply', 'switch', 'reset', 'scenario']) byId(id).disabled = disabled;
  if (state.data) {
    if (!byId('scenario').options.length) {
      for (const name of state.scenarios) byId('scenario').add(new Option(name, name));
      byId('scenario').value = state.scenario;
    }
    render(state.data);
  }
  if (state.error) byId('error').textContent = state.error;
}

async function operate(action, payload, clearDraft = false) {
  if (busy) return;
  busy = true;
  for (const id of ['enabled', 'fields', 'apply', 'switch', 'reset', 'scenario']) byId(id).disabled = true;
  byId('error').textContent = '';
  try {
    const response = await fetch('/api/test/' + action, {
      method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify(payload),
    });
    const result = await response.json();
    if (!response.ok) throw new Error(result.error);
    if (clearDraft) dirty.clear();
  } catch (error) {
    byId('error').textContent = error.message;
  } finally {
    busy = false;
    await refresh();
  }
}

byId('enabled').addEventListener('change', event => operate('enabled', {enabled: event.target.checked}));
byId('switch').addEventListener('click', () => operate('scenario', {scenario: byId('scenario').value}, true));
byId('reset').addEventListener('click', () => operate('reset', {}, true));
byId('form').addEventListener('submit', event => {
  event.preventDefault();
  const changes = {};
  for (const key of dirty) {
    const [group, field] = key.split('.');
    const input = inputs.get(key);
    (changes[group] ||= {})[field] = input.type === 'checkbox' ? input.checked : Number(input.value);
  }
  operate('apply', {changes}, true);
});
async function poll() {
  try { if (!busy) await refresh(); }
  catch (error) { byId('status').textContent = '连接失败'; }
  setTimeout(poll, 500);
}
poll();

const jsonData = {
  ssid: "",
  pwd: "",
  "cloud-email": "",
  "cloud-region": "",
  "cloud-username": "",
  "cloud-authToken": "",
};

const $id = document.getElementById.bind(document);
let refreshDone = false;

// Function to validate form inputs
function validateForm() {
  const form = document.querySelector("form");
  let isValid = true;
  const validationMessages = {
    ssid: "SSID is required.",
    password: "Password is required.",
    ip: "IP address is required.",
  };
  form.querySelectorAll("input").forEach((input) => {
    const errorElement = $id(input.name + "-error");
    if (input.value.trim() === "") {
      errorElement.textContent = validationMessages[input.name];
      isValid = false;
    } else {
      errorElement.textContent = "";
    }
  });

  return isValid;
}

// Function to refresh the tab
async function refreshTab() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });
  if (tab) {
    return new Promise((resolve) => {
      chrome.tabs.reload(tab.id, {}, () => {
        setTimeout(() => {
          resolve();
        }, 2000);
      });
    });
  }
}

async function isExtensionBlocked() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });

  return new Promise((resolve) => {
    if (tab.url) {
      return chrome.cookies.getAll({}, async (cookies) => {
        console.log(cookies);

        if (cookies.length === 0) {
          $id("main-blocked").style.display = "block";
          $id("main-loading").style.display = "none";
          resolve(true);
        } else {
          resolve(false);
        }
      });
    }
    return false;
  });
}
// Function to fetch metadata from the current tab
async function fetchMetadata() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });

  // Get SSID and password from the input fields
  const ssidValue = $id("ssid").value.trim();
  const pwdValue = $id("password").value.trim();

  // Update jsonData with ssid and password
  jsonData.ssid = ssidValue;
  jsonData.pwd = pwdValue;

  jsonData["cloud-region"] = tab.url.includes("bambulab.cn")
    ? "China"
    : "World";

  chrome.scripting.executeScript(
    {
      target: { tabId: tab.id },
      func: () => {
        const nextDataScript = document.getElementById("__NEXT_DATA__");
        if (nextDataScript && nextDataScript.type === "application/json") {
          const data = JSON.parse(nextDataScript.textContent);
          const userData = data?.props?.pageProps?.session?.user;
          if (userData) {
            return userData;
          } else {
            console.log("NOT USER DATA");
            console.log(nextDataScript.textContent);
            return { error: true };
          }
        } else {
          console.log("SCRIPT NOT FOUND");
          return { error: true };
        }
      },
    },
    (results) => {
      if (chrome.runtime.lastError) {
        $id("region-selector").style.display = "none";
        console.log("No user data found");
        console.log(chrome.runtime.lastError);
        toggleContainers(false);
        return;
      }

      const result = results[0]?.result;

      if (result && result.error) {
        console.log("No user data found");
        $id("region-selector").style.display = "none";
        toggleContainers(false);
        return;
      }

      if (!result || !result.account || !result.uidStr) {
        console.log("No user data found");
        $id("region-selector").style.display = "none";
        toggleContainers(false);
        return;
      }
      console.log("User data found");
      jsonData["cloud-email"] = result.account;
      jsonData["cloud-username"] = "u_" + result.uidStr;

      toggleContainers(true);
      fetchCookies();
    }
  );
}

// Function to fetch cookies
async function fetchCookies() {
  const [tab] = await chrome.tabs.query({ active: true, currentWindow: true });

  jsonData["cloud-region"] = tab.url.includes("bambulab.cn")
    ? "China"
    : "World";

  if (tab.url) {
    chrome.cookies.getAll({}, async (cookies) => {
      console.log("cookies", cookies);
      const filteredCookies = cookies.filter(
        (e) => e.domain.includes("bambulab.c") && e.name === "token"
      );
      console.log("filteredCookies", filteredCookies);
      if (filteredCookies.length === 0) {
        toggleContainers(false);
      } else {
        const authToken = filteredCookies[0].value;
        jsonData["cloud-authToken"] = authToken;
        $id("downloadJson").style.display = "inline-block";
        $id("downloadFilamentZip").style.display = "inline-block";
      }
    });
  }
}

/** printer は @BBL の直後のモデル名（P1P, X1C, A1, H2C, H2D など）。指定なし／all なら全件。 */
function filamentMatchesPrinter(name, printer) {
  if (!printer || printer === "all") return true;
  return name.indexOf("@BBL " + printer) >= 0;
}

function normalizeBrand(brand) {
  if (brand.startsWith("Generic")) return "Generic";
  if (brand.startsWith("Bambu")) return "Bambu Lab";
  return brand;
}

/** オブジェクトツリー内から最初に見つかった「数値っぽい」フィールドを再帰的に探す。
 * 値は number / string / [..] のどれでもよい。最初に解釈できた数値を返す。
 */
function findNumberFieldDeep(obj, key) {
  if (!obj || typeof obj !== "object") return null;
  if (Object.prototype.hasOwnProperty.call(obj, key)) {
    const v = obj[key];
    // そのまま number
    if (typeof v === "number" && Number.isFinite(v)) return v;
    // 文字列の場合
    if (typeof v === "string") {
      const n = parseFloat(v);
      if (Number.isFinite(n)) return n;
    }
    // 配列の場合（最初に解釈できた要素を使う）
    if (Array.isArray(v)) {
      for (const e of v) {
        if (typeof e === "number" && Number.isFinite(e)) return e;
        if (typeof e === "string") {
          const n = parseFloat(e);
          if (Number.isFinite(n)) return n;
        }
      }
    }
  }
  for (const k in obj) {
    if (!Object.prototype.hasOwnProperty.call(obj, k)) continue;
    const v = obj[k];
    if (v && typeof v === "object") {
      const found = findNumberFieldDeep(v, key);
      if (found !== null && found !== undefined) return found;
    }
  }
  return null;
}

/** 詳細 API の detail から MQTT tray_type 用の文字列を取得（例: "PLA Matte"）。最大15文字。 */
function getTrayTypeFromDetail(detail) {
  if (!detail || typeof detail !== "object") return "";
  let s = "";
  if (detail.filament_type != null) {
    if (typeof detail.filament_type === "string" && detail.filament_type.trim()) s = detail.filament_type.trim();
    else if (Array.isArray(detail.filament_type) && detail.filament_type.length > 0) {
      const first = detail.filament_type[0];
      if (typeof first === "string" && first.trim()) s = first.trim();
    }
  }
  if (!s && detail.setting && detail.setting.filament_type != null) {
    if (typeof detail.setting.filament_type === "string" && detail.setting.filament_type.trim()) s = detail.setting.filament_type.trim();
    else if (Array.isArray(detail.setting.filament_type) && detail.setting.filament_type.length > 0) {
      const first = detail.setting.filament_type[0];
      if (typeof first === "string" && first.trim()) s = first.trim();
    }
  }
  if (!s && typeof detail.name === "string" && detail.name.trim()) {
    const name = detail.name.trim();
    const atPos = name.indexOf(" @");
    const head = atPos >= 0 ? name.substring(0, atPos).trim() : name;
    const sp = head.indexOf(" ");
    s = sp >= 0 ? head.substring(sp + 1).trim() : head;
    if (s === "Lab") s = "";
  }
  if (typeof s !== "string" || !s) return "";
  return s.length > 15 ? s.substring(0, 15) : s;
}

/** slicer JSON の filament セクションから setting_id 一覧を集める。 */
function collectSettingIdsFromSlicer(slicerJson) {
  const filament = slicerJson.filament;
  if (!filament) return [];
  const pub = filament.public || [];
  const priv = filament.private || [];
  const ids = new Set();
  [].concat(pub, priv).forEach((entry) => {
    if (!entry) return;
    const sid = entry.setting_id || entry.id;
    if (!sid) return;
    ids.add(String(sid));
  });
  return Array.from(ids);
}

/** 詳細API(GET /v1/iot-service/api/slicer/setting/{SETTING_ID})を叩いて、setting_id ごとの温度範囲マップを構築する。
 *  tempsMap[sid] = { min, max, filament_id, tray_type } 形式。
 *  settingIds: filaments_*.txt に含まれる setting_id の配列（この対象だけ JSON を作る）。
 */
async function buildTempsMapFromCloud(host, token, settingIds) {
  if (!settingIds || settingIds.length === 0) return null;
  const tempsBySettingId = {};
  const total = settingIds.length;
  const concurrency = Math.min(3, total); // レート制限回避のため一度に 3 本まで fetch
  let nextIndex = 0;
  let completed = 0;

  const progressEl = document.getElementById("temps-progress");
  const zipBtn = document.getElementById("downloadFilamentZip");
  if (progressEl) {
    progressEl.style.display = "block";
    progressEl.textContent = `Temps: 0/${total}`;
  }
  if (zipBtn) {
    zipBtn.textContent = `Fetching temps... (0/${total})`;
  }

  async function worker(workerId) {
    while (true) {
      const i = nextIndex++;
      if (i >= total) break;
      const sid = settingIds[i];
      console.log(`[xtouch28] [w${workerId}] fetching setting detail ${i + 1}/${total}: ${sid}`);
      try {
        const res = await fetch(host + "/v1/iot-service/api/slicer/setting/" + encodeURIComponent(sid), {
          headers: { Authorization: "Bearer " + token },
        });
        if (!res.ok) continue;
        const detail = await res.json();
        const low = findNumberFieldDeep(detail, "nozzle_temperature_range_low");
        const high = findNumberFieldDeep(detail, "nozzle_temperature_range_high");
        let min = typeof low === "number" ? low : 0;
        let max = typeof high === "number" ? high : 0;
        // 範囲がない場合は nozzle_temperature を単一値として使う
        const single = findNumberFieldDeep(detail, "nozzle_temperature");
        if (min <= 0 && max <= 0 && typeof single === "number") {
          min = single;
          max = single;
        }
        if (min <= 0 && max <= 0) continue;
        const filamentId = detail.filament_id ? String(detail.filament_id) : "";
        const trayType = getTrayTypeFromDetail(detail);
        tempsBySettingId[sid] = { min, max, filament_id: filamentId, tray_type: trayType };
      } catch (e) {
        console.warn("[xtouch28] Failed to fetch setting detail for", sid, e);
      } finally {
        completed++;
        if (completed % 10 === 0 || completed === total) {
          console.log(`[xtouch28] temps progress: ${completed}/${total}`);
          if (progressEl) {
            progressEl.textContent = `Temps: ${completed}/${total}`;
          }
          if (zipBtn) {
            zipBtn.textContent = `Fetching temps... (${completed}/${total})`;
          }
        }
      }
    }
  }

  const workers = [];
  for (let w = 0; w < concurrency; w++) {
    workers.push(worker(w + 1));
  }
  await Promise.all(workers);
  console.log("[xtouch28] temps map built. entries:", Object.keys(tempsBySettingId).length);
  if (progressEl) {
    progressEl.textContent = `Temps: ${completed}/${total} (done)`;
  }
  if (zipBtn) {
    zipBtn.textContent = "Download filaments ZIP (xtouch/filament/)";
  }
  return tempsBySettingId;
}

function processOneEntry(entry, printer) {
  const name = entry.name || "";
  const sid = entry.setting_id || "";
  const fid = entry.filament_id != null ? String(entry.filament_id) : sid;
  if (!name) return null;
  if (printer && printer !== "all" && !filamentMatchesPrinter(name, printer)) return null;
  /* ノズルサイズは判定しない（まとめて1セット） */
  const atPos = name.indexOf(" @");
  const head = atPos >= 0 ? name.substring(0, atPos).trim() : name.trim();
  const sp = head.indexOf(" ");
  let brand = sp >= 0 ? head.substring(0, sp) : head;
  let type = sp >= 0 ? head.substring(sp + 1) : "";
  if (brand === "Bambu") {
    brand = "Bambu Lab";
    if (type.startsWith("Lab ")) type = type.substring(4);
  } else {
    brand = normalizeBrand(brand);
  }
  if (!brand) brand = "Other";
  if (!type) type = "Other";
  return { brand, filamentId: fid, entry: { id: sid, n: name, t: type } };
}

/** filament_id ごとに配列で最初に出現した setting_id のみ採用。ノズル判定なし・1セットにまとめる。 */
function buildPublicFilamentsFromSlicer(slicerJson, printer) {
  const filament = slicerJson.filament;
  if (!filament) return null;
  const pub = filament.public || [];
  const priv = filament.private || [];
  const brandsSet = new Set();
  const typesByBrand = {};
  const seenByBrand = {};
  [].concat(pub, priv).forEach((entry) => {
    const r = processOneEntry(entry, printer || "all");
    if (!r) return;
    if (!seenByBrand[r.brand]) seenByBrand[r.brand] = new Set();
    if (seenByBrand[r.brand].has(r.filamentId)) return;
    seenByBrand[r.brand].add(r.filamentId);
    brandsSet.add(r.brand);
    if (!typesByBrand[r.brand]) typesByBrand[r.brand] = [];
    typesByBrand[r.brand].push(r.entry);
  });
  // 各ブランド内のみ ID（setting_id）順でソート（ブランドの並びは変更しない）
  Object.keys(typesByBrand).forEach((brand) => {
    const arr = typesByBrand[brand] || [];
    arr.sort((a, b) => (a.id || "").localeCompare(b.id || "", undefined, { numeric: true }));
  });
  return { brands: Array.from(brandsSet), items: typesByBrand };
}

/** Build pipe format: brand1\nbrand2\n%%\nid|n|t\n... (per brand block). */
function buildPipeFormat(data) {
  if (!data || !data.brands || !data.items) return "";
  const lines = data.brands.slice();
  lines.push("%%");
  data.brands.forEach((brand) => {
    const arr = data.items[brand] || [];
    arr.forEach((it) => lines.push([it.id || "", it.n || "", it.t || ""].join("|")));
    lines.push("%%");
  });
  return lines.join("\n");
}

/** ブランド名をファイル名用に変換（スペース→アンダースコア、長さ制限）。ファームと一致させる。 */
function sanitizeBrandForFilename(brand) {
  if (!brand) return "Other";
  const s = String(brand).replace(/\s+/g, "_").replace(/[^A-Za-z0-9_-]/g, "_");
  return s.length > 20 ? s.slice(0, 20) : s;
}

async function downloadFilamentZip() {
  if (typeof JSZip === "undefined") {
    alert("JSZip を読み込めません。xtouch28 フォルダに jszip.min.js を置いてください。（https://unpkg.com/jszip@3.10.1/dist/jszip.min.js）");
    return;
  }
  const token = jsonData["cloud-authToken"];
  if (!token) return;
  const host = jsonData["cloud-region"] === "China" ? "https://api.bambulab.cn" : "https://api.bambulab.com";
  $id("downloadFilamentZip").disabled = true;
  $id("downloadFilamentZip").textContent = "Fetching...";
  try {
    const res = await fetch(host + "/v1/iot-service/api/slicer/setting?version=2.0.0.0", {
      headers: { Authorization: "Bearer " + token },
    });
    if (!res.ok) throw new Error(res.status + " " + res.statusText);
    const slicerJson = await res.json();
    const zip = new JSZip();
    const filamentFolder = zip.folder("xtouch").folder("filament");

    // 既存: ブランド/フィラメント一覧（UI 用）
    const out = buildPublicFilamentsFromSlicer(slicerJson, "all");
    if (out && out.brands.length > 0) {
      filamentFolder.file("filaments_brands.txt", out.brands.join("\n"));
      out.brands.forEach((brand) => {
        const arr = out.items[brand] || [];
        const lines = arr.map((it) => [it.id || "", it.n || "", it.t || ""].join("|"));
        const fname = "filaments_" + sanitizeBrandForFilename(brand) + ".txt";
        filamentFolder.file(fname, lines.join("\n"));
      });
    }

    // filaments_*.txt に含めた setting_id だけ詳細取得して json フォルダに JSON を作る
    const settingIdsForTemps = out && out.brands ? [...new Set(out.brands.flatMap((b) => (out.items[b] || []).map((it) => it.id).filter(Boolean)))] : [];
    const tempsMap = await buildTempsMapFromCloud(host, token, settingIdsForTemps);
    if (tempsMap && Object.keys(tempsMap).length > 0) {
      const jsonFolder = filamentFolder.folder("json");
      Object.keys(tempsMap).forEach((sid) => {
        const t = tempsMap[sid] || {};
        const obj = {
          filament_id: typeof t.filament_id === "string" ? t.filament_id : "",
          nozzle_min: typeof t.min === "number" ? t.min : 0,
          nozzle_max: typeof t.max === "number" ? t.max : 0,
          tray_type: typeof t.tray_type === "string" ? t.tray_type : "",
        };
        const json = JSON.stringify(obj, null, 2);
        jsonFolder.file(sid + ".json", json);
      });
    }
    const blob = await zip.generateAsync({ type: "blob" });
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = "xtouch_filaments.zip";
    a.click();
    URL.revokeObjectURL(a.href);
    a.remove();
  } catch (e) {
    alert("Failed: " + e.message);
  }
  $id("downloadFilamentZip").disabled = false;
  $id("downloadFilamentZip").textContent = "Download filaments ZIP (xtouch/filament/)";
}

// Function to toggle visibility of main containers
function toggleContainers(showMain) {
  if (showMain) {
    $id("main-container").style.display = "block";
    $id("main-login-container").style.display = "none";
  } else {
    $id("main-container").style.display = "none";
    $id("main-login-container").style.display = "block";
  }
}

// Function to handle downloading JSON data
async function downloadJsonData() {
  if (!validateForm()) {
    return;
  }

  $id("provisionDevice").style.display = "block";

  jsonData.ssid = $id("ssid").value.trim();
  jsonData.pwd = $id("password").value.trim();

  localStorage.setItem(
    "jsonData",
    JSON.stringify({ ...jsonData, ip: $id("ip").value.trim() })
  );

  downloadProvisioningJson();
}

async function provisionDevice() {
  if (!validateForm()) {
    return;
  }
  const ipValue = $id("ip").value.trim();
  if (ipValue !== "0.0.0.0") {
    try {
      const response = await fetch(`http://${ipValue}/provision`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(jsonData),
      });

      if (!response.ok) {
        $id("provision-error").style.display = "block";
      } else {
        $id("provision-error").style.display = "none";
      }
    } catch (error) {
      $id("provision-error").style.display = "block";
    }
  }
}

// Function to download provisioning JSON
function downloadProvisioningJson() {
  const a = document.createElement("a");
  const file = new Blob([JSON.stringify(jsonData, null, 2)], {
    type: "application/json",
  });
  a.href = URL.createObjectURL(file);
  a.download = "provisioning.json";
  a.click();
  a.remove();
}

// Load data from localStorage if available
function loadStoredData() {
  const storedJsonData = localStorage.getItem("jsonData");

  if (storedJsonData) {
    const localJsonData = JSON.parse(storedJsonData);
    console.log(localJsonData);
    $id("ip").value = localJsonData.ip;
    $id("ssid").value = localJsonData.ssid;
    $id("password").value = localJsonData.pwd;
  }
}

// Initialize the script
function initialize() {
  $id("main-loading").style.display = "none";
  loadStoredData();
  fetchMetadata();
}

function redirectTab(url) {
  chrome.tabs.query({ active: true, currentWindow: true }).then((tab) => {
    chrome.tabs.update(tab.id, { url });
  });
}

function checkUrl() {
  return chrome.tabs
    .query({ active: true, currentWindow: true })
    .then((tabs) => {
      console.log(tabs[0].url);
      if (
        !tabs[0].url.includes("bambulab.c") ||
        tabs[0].url.includes("store.bambulab.c")
      ) {
        $id("main-login-container").style.display = "block";
        $id("main-loading").style.display = "none";
        return false;
      }
      return true;
    });
}

function main() {
  checkUrl().then((correctURL) => {
    console.log("correctURL", correctURL);
    if (correctURL) {
      isExtensionBlocked().then((isBlocked) => {
        console.log("isBlocked", isBlocked);
        if (isBlocked) {
          $id("main-login-container").style.display = "none";
          return;
        } else {
          refreshTab().then(() => {
            initialize();
          });
        }
      });
    }
  });
}

document.addEventListener("DOMContentLoaded", () => {
  $id("downloadJson").addEventListener("click", downloadJsonData);
  $id("downloadFilamentZip").addEventListener("click", downloadFilamentZip);
  $id("provisionDevice-button").addEventListener("click", provisionDevice);

  $id("region-china").addEventListener("click", () => {
    const newUrl = "https://bambulab.cn";
    chrome.tabs.query({ active: true, currentWindow: true }, (tabs) => {
      if (tabs.length > 0) {
        chrome.tabs.update(tabs[0].id, { url: newUrl }, () => {
          main();
        });
      }
    });
  });
  $id("region-world").addEventListener("click", () => {
    const newUrl = "https://bambulab.com";
    chrome.tabs.query({ active: true, currentWindow: true }, (tabs) => {
      if (tabs.length > 0) {
        chrome.tabs.update(tabs[0].id, { url: newUrl }, () => {
          main();
        });
      }
    });
  });
  main();
});

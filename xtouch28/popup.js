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
    const folder = zip.folder("xtouch").folder("nozzle");
    const out = buildPublicFilamentsFromSlicer(slicerJson, "all");
    if (out && out.brands.length > 0) {
      folder.file("filaments_brands.txt", out.brands.join("\n"));
      out.brands.forEach((brand) => {
        const arr = out.items[brand] || [];
        const lines = arr.map((it) => [it.id || "", it.n || "", it.t || ""].join("|"));
        const fname = "filaments_" + sanitizeBrandForFilename(brand) + ".txt";
        folder.file(fname, lines.join("\n"));
      });
    }
    const blob = await zip.generateAsync({ type: "blob" });
    const a = document.createElement("a");
    a.href = URL.createObjectURL(blob);
    a.download = "xtouch_filaments_nozzle.zip";
    a.click();
    URL.revokeObjectURL(a.href);
    a.remove();
  } catch (e) {
    alert("Failed: " + e.message);
  }
  $id("downloadFilamentZip").disabled = false;
  $id("downloadFilamentZip").textContent = "Download filaments ZIP (xtouch/nozzle/)";
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

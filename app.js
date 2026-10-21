let parking = null;
const $ = (id) => document.getElementById(id);

function showMessage(text, type = "") {
  $("message").textContent = text;
  $("message").className = "message" + (type ? " " + type : "");
}

function refresh() {
  if (!parking) return;

  const states = parking.getSlots();
  $("available").textContent = parking.availableCount();
  $("occupied").textContent = parking.occupiedCount();
  $("waiting").textContent = parking.waitingCount();

  const slots = $("slots");
  slots.replaceChildren();

  for (let i = 0; i < states.size(); i++) {
    const occupied = states.get(i) === 1;
    const vehicle = parking.getSlotVehicle(i + 1);
    const card = document.createElement("div");
    card.className = "slot" + (occupied ? " occupied" : "");

    const top = document.createElement("div");
    top.className = "slot-top";

    const number = document.createElement("span");
    number.className = "slot-number";
    number.textContent = `SLOT P${i + 1}`;

    const icon = document.createElement("span");
    icon.className = "car-icon";
    icon.textContent = "🚘";

    const state = document.createElement("div");
    state.className = "slot-state";
    state.textContent = occupied ? "● Occupied" : "● Available";

    const reg = document.createElement("div");
    reg.className = "slot-vehicle";
    reg.textContent = vehicle || "Ready to use";

    top.append(number, icon);
    card.append(top, state, reg);
    slots.appendChild(card);
  }

  $("queue").textContent = parking.getWaitingQueue();
  $("exited").textContent = parking.getExitedStack();
  $("history").textContent = parking.getHistory();
}

function getVehicle() {
  return $("vehicle").value.trim();
}

function runVehicleAction(method) {
  if (!parking) {
    showMessage("C++ engine is still loading. Please wait.", "error");
    return;
  }

  const vehicle = getVehicle();
  if (!vehicle) {
    showMessage("Please enter a vehicle registration number.", "error");
    $("vehicle").focus();
    return;
  }

  const result = parking[method](vehicle);
  refresh();
  const isError = /already parked|not currently parked|not found|Please enter/i.test(result);
  showMessage(result, isError ? "error" : "success");
}

$("park").addEventListener("click", () => runVehicleAction("parkVehicle"));
$("exit").addEventListener("click", () => runVehicleAction("exitVehicle"));
$("search").addEventListener("click", () => runVehicleAction("searchVehicle"));
$("clear").addEventListener("click", () => {
  $("vehicle").value = "";
  $("vehicle").focus();
  showMessage("Input cleared.");
});
$("vehicle").addEventListener("keydown", (event) => {
  if (event.key === "Enter") runVehicleAction("parkVehicle");
});

async function startApp() {
  try {
    if (typeof createSmartParkModule !== "function") {
      throw new Error("smartpark.js is missing. Compile smartpark.cpp first.");
    }
    const Module = await createSmartParkModule();
    parking = new Module.SmartPark();
    $("status").textContent = "C++ engine ready";
    showMessage("SmartPark is ready. You can park, exit and search vehicles.", "success");
    refresh();
  } catch (error) {
    $("status").textContent = "Engine error";
    showMessage(error.message || String(error), "error");
    console.error(error);
  }
}

startApp();

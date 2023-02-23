function AJAX(a, e) {
    let c = d();
    c.onreadystatechange = b;

    function d() {
        if (window.XMLHttpRequest) {
            return new XMLHttpRequest()
        } else {
            if (window.ActiveXObject) {
                return new ActiveXObject("Microsoft.XMLHTTP")
            }
        }
    }

    function b() {
        if (c.readyState == 4) {
            if (c.status == 200) {
                if (e) {
                    e(c.responseText)
                }
            }
        }
    }

    this.doGet = function () {
        c.open("GET", a, true);
        c.send(null)
    };
    this.doPost = function (f) {
        c.open("POST", a, true);
        c.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        c.setRequestHeader("ISAJAX", "yes");
        c.send(f)
    }
}
function $(a) {return document.getElementById(a)}
function $$(a) {return document.getElementsByName(a)}
function $$_ie(a, c) {
    if (!a) {
        a = "*"
    }
    var b = document.getElementsByTagName(a);
    var e = [];
    for (let d = 0; d < b.length; d++) {
        att = b[d].getAttribute("name");
        if (att == c) {
            e.push(b[d])
        }
    }
    return e
}

/* начало моего кода*/
let login = '';
let password = '';
let DEBUG_MODE = window.location.protocol === 'file:'; // для отладки определяем запуск с http или c fs
function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
        currentDate = Date.now();
    } while (currentDate - date < milliseconds);
}

function data_load() {
    if (!DEBUG_MODE) {
        let oUpdate = new AJAX('/get_info.cgi', function (t) {
            result = JSON.parse(t);
            let namesList = ["device_mac_address", "mqtt_port", "mqtt_address", "mqtt_topic", "scada_address","mqtt_name","mqtt_password","modbus_speed"];
            for (i = 0; i < namesList.length; i++) {
                $(namesList[i]).value = result[namesList[i]];
            }
        });
        oUpdate.doGet();
    } else console.log("data_load() done");
}

function reboot() { /* перезагрузка */
    let oReboot = new AJAX('/reboot.cgi', function (t) {
    });
    oReboot.doGet();
    $('body_id').style.transition = "opacity 5s";
    $("body_id").style.opacity = "0";
    setTimeout(function () {
        $("body_id").style.opacity = "1";
        data_load()
    }, 5200);
}

function showDebbug() {
        let oDebbug = new AJAX('/debuginfo.cgi', function (t) {});
        oDebbug.doGet();
}

function send_form_() {
    if (!DEBUG_MODE) {
        let namesList = ["device_mac_address", "mqtt_port", "mqtt_address", "mqtt_topic", "scada_address", "login", "password" ,"mqtt_name","mqtt_password","modbus_speed"];
        let get_request = "/form.cgi?";
        for (let i = 0; i < namesList.length; i++) {
            get_request += namesList[i] + "=" + $(namesList[i]).value + "&";
        }
        let oSendForm = new AJAX(get_request, function (x) {
            setTimeout(function () {
                data_load()
            }, 500);
        });
        oSendForm.doGet();
    }
    ok_show('DATA SAVED');
}


function login_() {
    let login = $("login").value;
    $("login_new").value = login;
    password = $("password").value;
    if (DEBUG_MODE) {
        $("login_id").style.opacity = '0';
        setTimeout(function () {
            $("form_id").style.display = 'block';
            $("menu").style.display = 'flex';
            $('login_id').style.display = 'none';
            $("login_id").style.opacity = '1';
        }, 2000)
        return;
    }
    let oLogin = new AJAX('/login.cgi?login=' + login + '&password=' + password, function (t) {
        let result = JSON.parse(t);
        if (result.login == 1) {
            $("login_id").style.rotate = 'y 90deg';
            setTimeout(function () {
                $("form_id").style.display = 'block';
                $("menu").style.display = 'flex';
                $('login_id').style.display = 'none';
                $("login_id").style.rotate = 'y 0deg';
            }, 2000)
        } else {
            alert("Неверные данные авторизации!")
        }
    });
    oLogin.doGet();
}

function ok_show(msg) {
    $("ok_id").textContent = msg
    $('ok_id').style.top = '60vh'
    setTimeout(function (x) {
        $('ok_id').style.top = '50vh'
    }, 1000);
    setTimeout(function (x) {
        $('ok_id').style.top = '-5em'
    }, 4000);
}

function login_change_(save) {
    if (save) {
        let namesList = ["login_new", "password_new", "login", "password"];
        let get_request = "/form.cgi?";
        for (let i = 0; i < namesList.length; i++) {
            get_request += namesList[i] + "=" + $(namesList[i]).value + "&";
        }
        let oSendForm = new AJAX(get_request, function (t) {
            $("password_new").value = "";
        });
        if (DEBUG_MODE) (console.log(get_request)); else oSendForm.doGet();
    }
    let login = $("login_new").value;
    $("login").value = login;
    password = $("password_new").value;
    $("password").value = password;
    $('password_new').value = "";
    ok_show('NEW PASSWORD SAVED');
}

function menu_item_(number) {
    if (number != 4) {
        for (let x = 1; x < 7; x++) {
            if (x === number) {
                $("m_item_" + x).style.borderBottom = 'none'
            } else $("m_item_" + x).style.borderBottom = 'solid 1px'
        }
    }
    if (number === 1) {
        $("form_id").style.display = 'block';
        $("changePassword").style.display = 'none';
        $("log_console").style.display = 'none';
        $('modbus_window').style.display = 'none';
    }
    if (number === 2) {
        $("form_id").style.display = 'none';
        $("changePassword").style.display = 'none';
        $('modbus_window').style.display = 'none';
        $("log_console").style.display = 'block';
    }
    if (number === 3) {
        $("form_id").style.display = 'none';
        $("changePassword").style.display = 'flex';
        $("log_console").style.display = 'none';
        $('modbus_window').style.display = 'none';
    }
    if (number === 4) {
        reboot();
    }

    if (number === 5) {   // logout
        login = ''
        $("login_new").value = '';
        $("login").value = '';
        password = ''
        $("password").value = '';
        $("password_new").value = '';
        $("menu").style.display = 'none';
        $("login_id").style.display = 'none';
        $("form_id").style.display = 'none';
        $("changePassword").style.display = 'none';
        $("log_console").style.display = 'none';
        $('modbus_window').style.display = 'none';
        $('login_id').style.display = 'block';
    }

    if (number === 6) {   // modbus
        $("login_id").style.display = 'none';
        $("form_id").style.display = 'none';
        $("changePassword").style.display = 'none';
        $("log_console").style.display = 'none';
        $('login_id').style.display = 'none';
        $('modbus_window').style.display = 'block';
    }
}

let logString = ''
let mlogString = ''
let count = 0;

function load_log() {
    let now = new Date();
    if (DEBUG_MODE) {
        $("modbus_log_area").value= "123\n123";


    } else {
        let oLog = new AJAX('/log.cgi', function (result) {

            result = JSON.parse(result);

            if (result.log !== logString) {
                for (let i=0; i<100; i++)
                result.log = result.log.replace("__n__","\n");



                logString = result.log;
                $("log_area").value = '[' + now.toLocaleTimeString() + ']: ' + logString + '\n' + $('log_area').value;
            }
            result.mlog = result.mlog.replace("__n__","\n");
            result.mlog = result.mlog.replace("__n__","\n");
            result.mlog = result.mlog.replace("__n__","\n");

            if (result.mlog !== mlogString) {
                mlogString = result.mlog;
                $("modbus_log_area").value = '[' + now.toLocaleTimeString() + ']: ' + mlogString + '\n' + $("modbus_log_area").value;
            }


        });
        oLog.doGet();
    }
}

data_load();
setInterval(function f() { load_log(); }, 300);

/**  ModBusBlock  **/
function hexToSubstrings(hexArr) {
    return hexArr.map(str => [str.slice(0, 2), str.slice(2, 4), str.slice(4, 8), str.slice(8, 12)]);

}

function substringsToHex(substringArr) {
    return substringArr.map(subArr => subArr.join(''));
}
let   modbusCommandList = ["01050100FF00", "0105020012A2", "0202000212A2"];
let   modbusCommandListView =  [];
let     modbusInnerHTML = '';
const   modbusArea = $('modbusArea');

function mb_check(id,n) {
        $(id).value = $(id).value.replace(/[^0-9a-fA-F]/g, '');
        while ($(id).value.length < n) $(id).value = '0' + $(id).value
}

function modbusValueChange(id,i){
    console.log('mb'+id+'-'+i )
    modbusCommandListView[i][id]= $('mb'+id+'-'+i  ).value;
    modbusCommandList= substringsToHex(modbusCommandListView);
}

function modbusSend(id) {
        let get_request = "/modbus.cgi?modbuscommand=" + modbusCommandList[id];
        let oSendForm = new AJAX(get_request, function (t) {
        });
        oSendForm.doGet();
}

function modbusAddRow() {
    modbusCommandListView.push(["01","01","0000","0001"]);
    modbusCommandList= substringsToHex(modbusCommandListView);
    modbusRender();
}

function modbusRead(){
    let get_request = "/modbus.cgi" ;
    if (!DEBUG_MODE) {
        let oSendForm = new AJAX(get_request, function (t) {
            modbusCommandList = t.split("_")
            if (modbusCommandList[0] === "") modbusCommandList = [];
            modbusRender();
        });oSendForm.doGet();
    } else  modbusRender();


}

function modbusSave() {
    if (!DEBUG_MODE) {
        let saveString = modbusCommandList.join("_");
        if (saveString.length === 0) saveString = '0';
        if (saveString.length === 0) {
            saveString = "00"
        }
        let get_request = "/modbus.cgi?modbusprogram=" + saveString;
        let oSendForm = new AJAX(get_request, function (t) {
            modbusCommandList = t.split("_")
            if (modbusCommandList[0] === "") modbusCommandList = [];
            modbusRender();
            ok_show('DATA SAVED');
        });
        oSendForm.doGet();
    } else  ok_show('DATA SAVED');
}


function modbusRender() {
    modbusCommandListView =  hexToSubstrings(modbusCommandList);
    modbusInnerHTML = '<tr>' +
        '<td>##</td>' +
        '<td>Operation</td>' +
        '<td>Device (hex)</td>' +
        '<td>Address (hex)</td>' +
        '<td>Data (hex)</td>' +
        '<td></td>' +
        '<td></td>' +

        '</tr>';

    for (let i = 0; i < modbusCommandListView.length; i++) {
        modbusInnerHTML += `<tr><td>${i+1}.</td><td>
        
            <select id="mb1-${i}" onchange="modbusValueChange(1,${i})">
                <option value="01" ${(modbusCommandListView[i][1]=="01")?"selected":""}>Read Coil Status</option>
                <option value="02" ${(modbusCommandListView[i][1]=="02")?"selected":""}>Read Discrete Inputs</option>
                <option value="03" ${(modbusCommandListView[i][1]=="03")?"selected":""}>Read Holding Register</option>
                <option value="04" ${(modbusCommandListView[i][1]=="04")?"selected":""}>Read Input Registers</option>
                <option value="05" ${(modbusCommandListView[i][1]=="05")?"selected":""}>Force Single Coil</option>
                <option value="06" ${(modbusCommandListView[i][1]=="06")?"selected":""}>Preset Single Register</option>
            </select></td><td>
            <input id="mb0-${i}" value="${modbusCommandListView[i][0]}"  size="3" maxlength="2" onchange='mb_check("mb0-${i}",2);modbusValueChange(0,${i})'>
            </td><td><input id="mb2-${i}" value="${modbusCommandListView[i][2]}"  size="5" maxlength="4" onchange='mb_check("mb2-${i}",4);modbusValueChange(2,${i})'>
            </td><td><input id="mb3-${i}" value="${modbusCommandListView[i][3]}"  size="5" maxlength="4" onchange='mb_check("mb3-${i}",4);modbusValueChange(3,${i})'>
            </td><td><button type="button" onclick="modbusSend(${i});">send</button>
            </td><td><button type="button" onclick="modbusRemoveRow(${i})" class="redButton">remove</button></td>
            </tr>`; }
    modbusArea.innerHTML = `<table> ${modbusInnerHTML} </table> <br> ${(modbusCommandListView.length<50)?'<button onclick="modbusAddRow()">Add command</button>':''} 
<button onclick="modbusSave()">Save program</button><button onclick="modbusRead(); ok_show('DONE');">Load from device</button>
<button onclick=" $('modbus_log_area').value = ''">Clear log</button>`;
}
modbusRead();
function modbusRemoveRow(index) {
    modbusCommandList.splice(index, 1);
    modbusRender();
}
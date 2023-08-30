// It takes four arguments. The verb, which defines the HTTP verb to be used (GET, POST, PUT, DELETE).
// The second paramater is the BASE address (e.g. ‘http://localhost:5000 (opens new window)’).
// The third parameter is the endpoint to be used as a postfix to the BASE address
// (e.g. ‘http://localhost:5000/colors (opens new window)’). The fourth parameter is the optional obj,
// to be sent as JSON data to the service. The last parameter defines a callback to be called when
// the response returns. The callback receives a response object with the response data. Before we send
// the request, we indicate that we send and accept JSON data by modifying the request header.
// Credit: https://www.qt.io/product/qt6/qml-book/ch13-networking-rest-api
function request(verb, BASE, endpoint, obj, cb) {
//    console.log('request: ' + verb + ' ' + BASE + (endpoint ? '/' + endpoint : ''));
    var xhr = new XMLHttpRequest()
    xhr.onreadystatechange = function() {
        if(xhr.readyState === XMLHttpRequest.DONE) {
            if (xhr.status === 200) {
                if(cb) {
                    var res = JSON.parse(xhr.responseText.toString())
                    res.error = null;
                    cb(res)
                }
            } else {
                console.log('Error in request: ' + verb + ' ' + BASE + (endpoint ? '/' + endpoint : ''));
                var errorResponse;
                try {
                    errorResponse = JSON.parse(xhr.responseText.toString());
                } catch (err) {
                    errorResponse = {error: "error"};
                }
                cb(errorResponse);
            }
        }
    }
    xhr.open(verb, BASE + (endpoint ? '/' + endpoint : ''))
    xhr.setRequestHeader('Content-Type', 'application/json')
    xhr.setRequestHeader('Accept', 'application/json')
    var data = obj ? JSON.stringify(obj) : ''
    xhr.send(data)
}

// Copyright (c) 2020 Professor Peanut
// MIT license see accompanying file 'nanopulsedb.h' for details

// import visualization libraries {
const { Tracer, Array1DTracer, Array2DTracer, LogTracer, Layout, VerticalLayout } = require('algorithm-visualizer');
// }

var ni = [  0,  0,  0,  0,  0,  0,  0];
var nf = [  0,  0,  0,  0,  0,  0,  0];
var nn = [  [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0],
            [0,0,0,0,0,0,0] ];
var vv = [  0,  0,  0,  0,  0,  0,  0];
const N = ni.length;

// define tracer variables {
const tracerids = new Array1DTracer('ids');
const tracerfile = new Array1DTracer('filepos');
const tracernext = new Array2DTracer('next');
const tracervis = new Array1DTracer('visits');
const logger = new LogTracer('Console');
Layout.setRoot(new VerticalLayout([tracerids,
                                   tracerfile,
                                   tracernext,
                                   tracervis,
                                   logger]));
tracerids.set(ni);
tracerfile.set(nf);
tracernext.set(nn);
tracervis.set(vv);
Tracer.delay();
// }

var heads = [0,0,0,0];
var visits = [0,0,0,0];
var nKeys = 0;//[0,0,0,0];
var nBottomLen = 0;

function getNewNodePos() {
    return nKeys++;
}

function insertSkipnode(key, filepos, layer) {
    const newNodeAddr = getNewNodePos();
    // visualize {
    logger.println(`new node addr: ${newNodeAddr}`);
    logger.println(`nKeys: ${nKeys}`);
    tracerids.select(newNodeAddr);
    tracerfile.select(newNodeAddr);
    tracernext.select(layer, newNodeAddr)
    Tracer.delay();
    // }
    ni[newNodeAddr] = key;
    nf[newNodeAddr] = filepos;
    // visualize {
    tracerids.patch(newNodeAddr, ni[newNodeAddr]);
    tracerfile.patch(newNodeAddr, nf[newNodeAddr]);
    Tracer.delay();
    tracerids.depatch(newNodeAddr);
    tracerfile.depatch(newNodeAddr);
    // }
    
    if (key < ni[heads[layer]]) {
        nn[layer][newNodeAddr] = heads[layer];
        // visualize {
        tracernext.patch(layer, newNodeAddr, heads[layer]);
        Tracer.delay();
        tracernext.depatch(layer, newNodeAddr);
        // }
        heads[layer] = newNodeAddr;
        return;
    }
    
    var current = heads[layer];
    var next = 0;
    
    for (var i=0; i<nKeys-2; i++) {
        next = nn[layer][current];
        // visualize {
        logger.println(`  current: ${current}`);
        logger.println(`  next: ${next}`);
        tracerids.deselect(0, N-1);
        tracerfile.deselect(0, N-1);
        tracernext.deselect(0, 0, 3, 6);
        tracerids.select(current);
        tracerfile.select(current);
        tracernext.select(layer, current);
        Tracer.delay();
        // }
        if (ni[next] > key) {
            // visualize {
            logger.println(`--- break ----`);
            Tracer.delay();
            // }
            nn[layer][newNodeAddr] = next;
            // visualize {
            tracernext.patch(layer, newNodeAddr, next);
            Tracer.delay();
            tracernext.depatch(layer, newNodeAddr);
            // }
            break;
        }
        current = next;
    }
    
    // visualize {
    logger.println(`  current: ${current}`);
    Tracer.delay();
    // }
    

    
    
    nn[layer][current] = newNodeAddr;
    // visualize {
    tracernext.patch(layer, current, newNodeAddr);
    Tracer.delay();
    tracernext.depatch(layer, current);
    // }
}

function promoteSkipnode(key, layer) {
    
}

function findSkipnode(key, layer) {
    var current = heads[layer];
    for (var i=0; i<nKeys; i++) {
        // visualize {
        tracerids.select(current);
        tracerfile.select(current);
        tracernext.select(layer, current);
        Tracer.delay();
        tracerids.deselect(0, N-1);
        tracerfile.deselect(0, N-1);
        tracernext.deselect(0, 0, 3, 6);
        // }
        if (/*ni[current] === 0 ||*/ layer <3 && ni[current] > key) {
            // overshoot or empty
            // visualize {
            tracervis.select(current);
            logger.println(`layer: ${layer}`);
            Tracer.delay();
            tracervis.deselect(0, N-1);
            // }
            return findSkipnode(key, layer+1);
        } else if (ni[current] === key) {
            vv[current] += 1;
            visits[layer] += 1;
            if (((vv[current]*100) / visits[layer]) > 20) {
                // promote:
                // visualize {
                logger.println(`promote: ${layer-1} `);
                Tracer.delay();
                // }
                insertSkipnode(key, /*nf[current]*/321, layer-1);
            }
            
            // visualize {
            tracervis.patch(current, vv[current]);
            Tracer.delay();
            tracervis.depatch(current);
            // }
            return current;
        }
        current = nn[layer][current];
    }
    return "not found";
}

function start() {
    insertSkipnode(500, 0, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(111, 1, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(444, 2, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(222, 3, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(333, 4, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(44, 5, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertSkipnode(555, 6, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    const resA = findSkipnode(123, 0);
    // visualize {
    logger.println(`resA (should be 'not found'): ${resA}`);
    // }
    const resB = findSkipnode(222, 0);
    // visualize {
    logger.println(`resB (should be 3): ${resB}`);
    // }
}

start();



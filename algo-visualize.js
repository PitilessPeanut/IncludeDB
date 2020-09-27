// Copyright (c) 2020 Professor Peanut
// MIT license see accompanying file 'includedb.h' for details

// import visualization libraries {
const { Tracer, Array1DTracer, Array2DTracer, LogTracer, Layout,
        HorizontalLayout, VerticalLayout } = require('algorithm-visualizer');
// }

var nodevec = [ {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    },
    {
    "id" : 0,
    "fileidx" : 0,
    "next" : [0,0,0,0],
    "visits" : 0
    } ];
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
Layout.setRoot(new VerticalLayout([new HorizontalLayout[tracerids,
                                                        tracerfile,
                                                        tracervis]),
                                   tracernext,
                                   logger]));
tracerids.set(ni);
tracerfile.set(nf);
tracernext.set(nn);
tracervis.set(vv);
Tracer.delay();
// }

var heads = [0,0,0,0];
var visits = 0;
var nKeys = 0;//[0,0,0,0];

function getNewNodePos() {
    return nKeys++;
}

function insertNewSkipnode(key, filepos) {
    const newNodeAddr = getNewNodePos();
    
    // visualize {
    logger.println(`new node addr: ${newNodeAddr}`);
    logger.println(`nKeys: ${nKeys}`);
    tracerids.select(newNodeAddr);
    tracerfile.select(newNodeAddr);
    Tracer.delay();
    // }
    
    nodevec[newNodeAddr].id = key;
    nodevec[newNodeAddr].fileidx = filepos;
    
    // visualize {
    tracerids.patch(newNodeAddr, nodevec[newNodeAddr].id);
    tracerfile.patch(newNodeAddr, nodevec[newNodeAddr].fileidx);
    Tracer.delay();
    tracerids.depatch(newNodeAddr);
    tracerfile.depatch(newNodeAddr);
    // }
    
    insertSkipnode(key, newNodeAddr, 3);
}

function insertSkipnode(key, pos, layer) {
    // visualize {
    tracernext.select(layer, pos);
    // }
    
    if (key < nodevec[heads[layer]].id) {
        nodevec[pos].next[0] = pos;
        nodevec[pos].next[1] = pos;
        nodevec[pos].next[2] = pos;
        nodevec[pos].next[layer] = heads[layer];
        // visualize {
        tracernext.patch(0, pos, nodevec[pos].next[0]);
        tracernext.patch(1, pos, nodevec[pos].next[1]);
        tracernext.patch(2, pos, nodevec[pos].next[2]);
        tracernext.patch(layer, pos, nodevec[pos].next[layer]);
        Tracer.delay();
        tracernext.depatch(0, pos);
        tracernext.depatch(1, pos);
        tracernext.depatch(2, pos);
        tracernext.depatch(layer, pos);
        // }
        heads[0] = pos;
        heads[1] = pos;
        heads[2] = pos;
        heads[layer] = pos;
        return;
    }
    
    var current = heads[layer];
    var next = 0;
    
    for (var i=0; i<nKeys-2; i++) {
        next = nodevec[current].next[layer];
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
        if (nodevec[next].id > key) {
            // visualize {
            logger.println(`--- break ----`);
            Tracer.delay();
            // }
            nodevec[pos].next[0] = pos;
            nodevec[pos].next[1] = pos;
            nodevec[pos].next[2] = pos;
            nodevec[pos].next[layer] = next;
            // visualize {
            tracernext.patch(0, pos, nodevec[pos].next[0]);
            tracernext.patch(1, pos, nodevec[pos].next[1]);
            tracernext.patch(2, pos, nodevec[pos].next[2]);
            tracernext.patch(layer, pos, next);
            Tracer.delay();
            tracernext.depatch(0, pos);
            tracernext.depatch(1, pos);
            tracernext.depatch(2, pos);
            tracernext.depatch(layer, pos);
            // }
            break;
        }
        else if (current === next) {
            // visualize {
            logger.println(`STOP`);
            Tracer.delay();
            // }
            i = nKeys; // break
        }
        current = next;
    }
    
    // visualize {
    logger.println(`  current: ${current}`);
    Tracer.delay();
    // }
    

    
    
    nodevec[current].next[layer] = pos;
    // visualize {
    tracernext.patch(layer, current, pos);
    Tracer.delay();
    tracernext.depatch(layer, current);
    // }
}

function findSkipnode(key) {
    const res = findPrevSkipnode(key, heads[0], 0);
    // visualize {
        logger.println(`res: ${res} key: ${key}`);
        Tracer.delay();
    // }
    return nodevec[res].id === key ? res : "not found";
}

function findPrevSkipnode(key, start, layer) {
    var current = start;
    var prev = current;
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
        if (layer <3 && (prev===current || nodevec[current].id > key)) {
            // overshoot or empty
            // visualize {
            tracervis.select(current);
            logger.println(`layer: ${layer} nodevec[cur].id ${nodevec[current].id} prev ${prev} cur ${current}`);
            Tracer.delay();
            tracervis.deselect(0, N-1);
            // }
            return findPrevSkipnode(key, nodevec[current].next[layer+1], layer+1);
        } else if (nodevec[current].id === key) {
            nodevec[current].visits += 1;
            visits += 1;
            if (((nodevec[current].visits*100) / visits) > 20) {
                // promote:
                // visualize {
                logger.println(`promote new layer: ${layer-1} cur: ${current}`);
                Tracer.delay();
                // }
                insertSkipnode(key, current, layer-1);
            }
            
            // visualize {
            tracervis.patch(current, nodevec[current].visits);
            Tracer.delay();
            tracervis.depatch(current);
            // }
            return current;
        } else if (nodevec[current].id >= key) {
            // visualize {
            logger.println(`stopping early. cur hay: ${current} needle ${key}`);
            Tracer.delay();
            // }
            break;
        }
        prev = current;
        current = nodevec[current].next[layer];
    }
    return prev;
}

function start() {
    insertNewSkipnode(500, 0);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(111, 1);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(444, 2);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(222, 3);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(333, 4);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(44, 5);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    insertNewSkipnode(555, 6);
    // visualize {
    tracerids.deselect(0, N-1);
    tracerfile.deselect(0, N-1);
    tracernext.deselect(0, 0, 3, 6);
    // }
    // visualize {
    logger.println(` --- Searching ---`);
    // }
    const resA = findSkipnode(123);
    // visualize {
    logger.println(`resA (should be 'not found'): ${resA}`);
    Tracer.delay();
    // }
    const resB = findSkipnode(222);
    // visualize {
    logger.println(`resB (should be 3): ${resB}`);
    Tracer.delay();
    // }
    const resC = findSkipnode(333);
    // visualize {
    logger.println(`resC(should be 4): ${resC}`);
    Tracer.delay();
    // }
}

start();



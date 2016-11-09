package ast;

import pdg.PDGNode;

import java.util.Collection;
import java.util.HashMap;
import java.util.Map;

/**
 * Created by ooee on 11/8/16.
 */
public class VariableDeclarationMap {
    private Map<String, PDGNode<VariableDeclaration>> map;

    public VariableDeclarationMap() {
        map = new HashMap<>();
    }

    public boolean containsKey(String string) {
        if (string.contains(".")) {
            String rootIdentifier = string.split("\\.")[0];
            return map.containsKey(rootIdentifier);
        }
        return map.containsKey(string);
    }

    public PDGNode<VariableDeclaration> get(String string) {
        String rootIdentifier = string.split("\\.")[0];
        return map.get(rootIdentifier);
    }

    public void put(String variableName, PDGNode<VariableDeclaration> pdgNode) {
        map.put(variableName, pdgNode);
    }

    public Collection<PDGNode<VariableDeclaration>> values() {
        return map.values();
    }
}

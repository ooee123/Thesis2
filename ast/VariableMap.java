package ast;

import ast.type.StructUnionType;
import ast.type.Type;
import ast.type.TypedefType;
import com.google.common.collect.Sets;
import pdg.PDGNode;

import java.util.*;

/**
 * Created by ooee on 11/7/16.
 */
public class VariableMap {

    private Map<String, Collection<PDGNode<? extends BlockItem>>> variableMap;

    public VariableMap() {
        variableMap = new HashMap<>();
    }

    /**
     * If I'm asking if a.b is in by checking a.b and a as well.
     * I'm asking if variableName is in any part of anyone in this VariableMap
     * @param variableName
     * @return
     */
    public boolean containsKey(String variableName) {
        //String variableName = identifier.getIdentifier();
        List<String> split = Arrays.asList(variableName.split("\\."));
        if (variableName.contains(".")) {
            for (int i = split.size(); i > 0; i--) {
                String key = String.join(".", split.subList(0, i));
                if (variableMap.containsKey(key)) {
                    return true;
                }
            }
        }
        for (String s : variableMap.keySet()) {
            if (s.equals(variableName) || s.startsWith(variableName + ".")) {
                return true;
            }
        }
        return false;
    }

    /**
     *
     * @param variableName
     * @param map
     * @return
     */
    public boolean containsKey(String variableName, Map<String, Type> map) {
        if (containsKey(variableName)) {
            return true;
        }
        List<String> split = Arrays.asList(variableName.split("\\."));
        Type type = map.get(split.get(0));
        while (type instanceof TypedefType) {
            type = ((TypedefType) type).getOriginalType();
        }
        for (String s : variableMap.keySet()) {
            if (s.startsWith(split.get(0)) && type instanceof StructUnionType) {
                if (((StructUnionType) type).getStructUnion().equals(StructUnionType.StructUnion.UNION)) {
                    return true;
                } else {
                    for (int i = 1; i < split.size(); i++) {
                        type = ((StructUnionType) type).getField(split.get(i));
                        while (type instanceof TypedefType) {
                            type = ((TypedefType) type).getOriginalType();
                        }
                        if (type instanceof StructUnionType && ((StructUnionType) type).getStructUnion().equals(StructUnionType.StructUnion.UNION)) {
                            /*
                            List<String> sSplit = Arrays.asList(s.split("\\."));
                            if (sSplit.subList(0, sSplit.size() - 1).equals(split.subList(0, split.size() - 1))) {
                                return true;
                            }
                            */
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    /**
     * Grabs the most specific level of change. If a.b is not available, return a.
     * @param variableName
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> get(String variableName) {
        //String variableName = identifier.getIdentifier();
        if (variableName.contains(".")) {
            List<String> split = Arrays.asList(variableName.split("\\."));
            for (int i = split.size(); i > 0; i--) {
                String key = String.join(".", split.subList(0, i));
                if (variableMap.containsKey(key)) {
                    return variableMap.get(key);
                }
            }
        }
        return variableMap.get(variableName);
    }

    public Collection<Collection<PDGNode<? extends BlockItem>>> getAllAssociated(String variableName) {
        //String variableName = identifier.getIdentifier();
        Collection<Collection<PDGNode<? extends BlockItem>>> associated = Sets.newIdentityHashSet();
        for (String s : variableMap.keySet()) {
            if (s.equals(variableName) || s.startsWith(variableName + ".")) {
                associated.add(variableMap.get(s));
            }
        }
        List<String> split = Arrays.asList(variableName.split("\\."));

        for (int i = split.size(); i > 0; i--) {
            String key = String.join(".", split.subList(0, i));
            if (variableMap.containsKey(key)) {
                associated.add(variableMap.get(key));
            }
        }
        return associated;
    }

    /**
     * Used to change variables. So if I want to modify the variable a, then a.b, a.c, etc. will also be changed.
     * Also get parents of a
     * @param variableName
     * @return Returns all variables that get modified if identifier is reassigned.
     */
    public Collection<Collection<PDGNode<? extends BlockItem>>> getAllAssociated(String variableName, Map<String, Type> map) {
        Collection<Collection<PDGNode<? extends BlockItem>>> associated = Sets.newIdentityHashSet();
        associated.addAll(getAllAssociated(variableName));
        List<String> split = Arrays.asList(variableName.split("\\."));
        String variableIdentifier = split.get(0);
        Type type = map.get(split.get(0));
        while (type instanceof TypedefType) {
            type = ((TypedefType) type).getOriginalType();
        }
        if (type instanceof StructUnionType) {
            if (((StructUnionType) type).getStructUnion().equals(StructUnionType.StructUnion.UNION)) {
                // All of variable is affected
                //associated.addAll(getAllAssociated(new PrimaryExpressionIdentifier(variableIdentifier, variableType)));
                associated.addAll(getAllAssociated(variableIdentifier));
            } else {
                for (int i = 1; i < split.size(); i++) {
                    type = ((StructUnionType) type).getField(split.get(i));
                    while (type instanceof TypedefType) {
                        type = ((TypedefType) type).getOriginalType();
                    }
                    if (type instanceof StructUnionType) {
                        if (((StructUnionType) type).getStructUnion().equals(StructUnionType.StructUnion.UNION) && i == split.size() - 2) {
                            //associated.addAll(getAllAssociated(new PrimaryExpressionIdentifier(variableIdentifier + "." + String.join(".", split.subList(1, i + 1)), field.getType()));
                            associated.addAll(getAllAssociated(variableIdentifier + "." + String.join(".", split.subList(0, i))));
                            i = split.size();
                        }
                    }
                }

            }
        }
        return associated;
    }

    public int size() {
        return variableMap.size();
    }

    /**
     * Specific o
     * @param o
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> get(Object o) {
        return variableMap.get(o);
    }

    /**
     * For all variables under s, create a new collection with just t
     * @param s
     * @param t
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> replace(String s, PDGNode<? extends BlockItem> t) {
        //String s = identifier.getIdentifier();
        Collection<PDGNode<? extends BlockItem>> ret = get(s);
        Set<PDGNode<? extends BlockItem>> set = Sets.newIdentityHashSet();
        set.add(t);
        variableMap.put(s, set);
        for (String s1 : variableMap.keySet()) {
            if (s1.equals(s) || s1.startsWith(s + ".")) {
                Set<PDGNode<? extends BlockItem>> set2 = Sets.newIdentityHashSet();
                set2.add(t);
                variableMap.put(s1, set2);
            }
        }
        return ret;
    }

    /**
     * For all variables under s, add t to that Collection. Also get parents of s.
     * @param s
     * @param t
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> augment(String s, PDGNode<? extends BlockItem> t) {
        //String s = identifier.getIdentifier();
        Collection<PDGNode<? extends BlockItem>> ret = get(s);
        if (!variableMap.containsKey(s)) {
            variableMap.put(s, Sets.newIdentityHashSet());
        }
        variableMap.get(s).add(t);
        for (String s1 : variableMap.keySet()) {
            if (!s1.equals(s) && s1.startsWith(s + ".")) { // Or or and?
                variableMap.putIfAbsent(s1, Sets.newIdentityHashSet());
                variableMap.get(s1).add(t);
            }
        }
        return ret;
    }

    public Collection<PDGNode<? extends BlockItem>> getAllPDGNodes() {
        Collection<PDGNode<? extends BlockItem>> pdgNodes = Sets.newIdentityHashSet();
        for (Collection<PDGNode<? extends BlockItem>> nodes : variableMap.values()) {
            pdgNodes.addAll(nodes);
        }
        return pdgNodes;
    }

    public void variablePotentiallyChanged(String identifier, PDGNode<? extends BlockItem> t) {
        Collection<Collection<PDGNode<? extends BlockItem>>> allAssociated = getAllAssociated(identifier);
        for (Collection<PDGNode<? extends BlockItem>> pdgNodes : allAssociated) {
            pdgNodes.add(t);
        }
    }

    public void variableGuaranteedChanged(String identifier, PDGNode<? extends BlockItem> t) {
        Collection<Collection<PDGNode<? extends BlockItem>>> allAssociated = getAllAssociated(identifier);
        for (Collection<PDGNode<? extends BlockItem>> pdgNodes : allAssociated) {
            pdgNodes.clear();
            pdgNodes.add(t);
        }
    }

    public void clearVariable(String variable) {
        for (Collection<PDGNode<? extends BlockItem>> pdgNodes : getAllAssociated(variable)) {
            pdgNodes.clear();
        }
    }

    public Set<String> keySet() {
        return variableMap.keySet();
    }

    public Collection<Collection<PDGNode<? extends BlockItem>>> values() {
        return variableMap.values();
    }
}

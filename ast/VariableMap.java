package ast;

import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import pdg.PDGNode;

import java.util.*;

/**
 * Created by ooee on 11/7/16.
 */
public class VariableMap implements Map<String, Collection<PDGNode<? extends BlockItem>>> {

    private Map<String, Collection<PDGNode<? extends BlockItem>>> variableMap;

    public VariableMap() {
        variableMap = new HashMap<>();
    }

    /**
     * If I'm asking if a.b is in by checking a.b and a as well.
     * @param variableName
     * @return
     */
    public boolean containsKey(String variableName) {
        if (variableName.contains(".")) {
            List<String> split = Arrays.asList(variableName.split("."));
            for (int i = split.size(); i > 0; i--) {
                String key = String.join(".", split.subList(0, i));
                if (variableMap.containsKey(key)) {
                    return true;
                }
            }
        }
        return variableMap.containsKey(variableName);
    }

    /**
     * Grabs the most specific level of change. If a.b is not available, return a.
     * @param variableName
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> get(String variableName) {
        if (variableName.contains(".")) {
            List<String> split = Arrays.asList(variableName.split("."));
            for (int i = split.size(); i > 0; i--) {
                String key = String.join(".", split.subList(0, i));
                if (variableMap.containsKey(key)) {
                    return variableMap.get(key);
                }
            }
        }
        return variableMap.get(variableName);
    }
    /**
     * Used to change variables. So if I want to modify the variable a, then a.b, a.c, etc. will also be changed.
     * @param variableName
     * @return
     */
    public Collection<Collection<PDGNode<? extends BlockItem>>> getAllAssociated(String variableName) {
        Collection<Collection<PDGNode<? extends BlockItem>>> associated = new ArrayList<>();
        for (String s : variableMap.keySet()) {
            if (s.startsWith(variableName)) {
                associated.add(variableMap.get(s));
            }
        }
        return associated;
    }

    @Override
    public int size() {
        return variableMap.size();
    }

    @Override
    public boolean isEmpty() {
        return variableMap.isEmpty();
    }

    /**
     * Strict o
     * @param o
     * @return
     */
    @Override
    public boolean containsKey(Object o) {
        return variableMap.containsKey(o);
    }

    @Override
    public boolean containsValue(Object o) {
        return variableMap.containsValue(o);
    }

    /**
     * Specific o
     * @param o
     * @return
     */
    @Override
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
        Collection<PDGNode<? extends BlockItem>> ret = get(s);
        Set<PDGNode<? extends BlockItem>> set = Sets.newIdentityHashSet();
        set.add(t);
        variableMap.put(s, set);
        for (String s1 : variableMap.keySet()) {
            if (s1.startsWith(s)) {
                Set<PDGNode<? extends BlockItem>> set2 = Sets.newIdentityHashSet();
                set2.add(t);
                variableMap.put(s1, set2);
            }
        }
        return ret;
    }

    /**
     * For all variables under s, add t to that Collection
     * @param s
     * @param t
     * @return
     */
    public Collection<PDGNode<? extends BlockItem>> augment(String s, PDGNode<? extends BlockItem> t) {
        Collection<PDGNode<? extends BlockItem>> ret = get(s);
        if (variableMap.containsKey(s)) {
            variableMap.put(s, Sets.newIdentityHashSet());
        }
        variableMap.get(s).add(t);
        for (String s1 : variableMap.keySet()) {
            if (s1.startsWith(s)) {
                if (variableMap.containsKey(s1)) {
                    variableMap.put(s1, Sets.newIdentityHashSet());
                }
                variableMap.get(s1).add(t);
            }
        }
        return ret;
    }

    @Override
    public Collection<PDGNode<? extends BlockItem>> remove(Object o) {
        throw new UnsupportedOperationException();
        //return variableMap.remove(o);
    }

    @Override
    public void putAll(Map<? extends String, ? extends Collection<PDGNode<? extends BlockItem>>> map) {
        throw new UnsupportedOperationException();
        //variableMap.putAll(map);
    }

    public void clearVariable(String variable) {
        for (Collection<PDGNode<? extends BlockItem>> pdgNodes : getAllAssociated(variable)) {
            pdgNodes.clear();
        }
    }

    @Override
    public void clear() {
        variableMap.clear();
    }

    @Override
    public Set<String> keySet() {
        return variableMap.keySet();
    }

    @Override
    public Collection<Collection<PDGNode<? extends BlockItem>>> values() {
        return variableMap.values();
    }

    @Override
    public Set<Entry<String, Collection<PDGNode<? extends BlockItem>>>> entrySet() {
        return variableMap.entrySet();
    }

    @Override
    public Collection<PDGNode<? extends BlockItem>> put(String s, Collection<PDGNode<? extends BlockItem>> nodes) {
        throw new UnsupportedOperationException();
    }
}

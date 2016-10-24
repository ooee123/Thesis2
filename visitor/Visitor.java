package visitor;

import ast.Statement;

import java.util.Collection;

/**
 * Created by ooee on 10/22/16.
 */
public interface Visitor<T, V> {
    Collection<T> visit(V v);
}

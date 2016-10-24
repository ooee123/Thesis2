package utils;

import java.util.AbstractSet;
import java.util.Iterator;

/**
 * Created by ooee on 10/22/16.
 */
public class IdentitySet<T> extends AbstractSet<T> {
    @Override
    public Iterator<T> iterator() {
        return null;
    }

    @Override
    public int size() {
        return 0;
    }

    @Override
    public boolean add(T t) {
        return super.add(t);
    }
}

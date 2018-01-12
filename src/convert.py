import sys
import xmltodict

in_path = sys.argv[1]
out_path = sys.argv[2]

with open(in_path) as infile:
    doc = xmltodict.parse(infile.read())

lista = doc['xsd:schema']['xsd:simpleType']['xsd:restriction']['xsd:enumeration']
with open(out_path, 'w') as outfile:
    for a in lista:
        outfile.write(a['@value'] + ' ' + a['xsd:annotation']['xsd:documentation'] + '\n')

